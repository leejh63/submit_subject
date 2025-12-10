#!/usr/bin/env python3
# 실행: python3 draw_val.py . -I include -I "$(gcc -print-file-name=include)" -D_GNU_SOURCE --std=c11 -o var_dig.drawio --debug

"""
draw_val.py ― 구조체·union·enum 세로 리스트 + 함수/변수 다이어그램 (.drawio)

• 구조체/union/enum 내부 필드/상수 제대로 표시
• 블록별 fillColor 고정, 항목은 흰색 배경
"""

import os
from clang import cindex
import argparse
import itertools
import pathlib
import zlib
import xml.etree.ElementTree as ET
import base64

# libclang.so 경로 설정
LIBCLANG_PATH = os.environ.get("LIBCLANG_PATH")
if LIBCLANG_PATH:
    cindex.Config.set_library_file(LIBCLANG_PATH)
else:
    cindex.Config.set_library_file(
        "/home/jaeholee/.local/lib/python3.10/site-packages/clang/native/libclang.so"
    )

# 전역 ID 생성기 및 함수
gen_id = itertools.count(2)
newid = lambda: str(next(gen_id))

# ----------------------------------------------------------------
# 파일 수집
# ----------------------------------------------------------------
def gather(paths, debug_mode):
    found_files = []
    for p in paths:
        p_path = pathlib.Path(p)
        if p_path.is_dir():
            for f in p_path.rglob("*"):
                if f.suffix in {".c", ".h", ".cpp", ".hpp"}:
                    found_files.append(str(f))
        elif p_path.suffix in {".c", ".h", ".cpp", ".hpp"}:
            found_files.append(str(p_path))
    if debug_mode:
        print(f"[DEBUG-Gather] Finished gathering. Total files found: {len(found_files)}")
    return found_files

# ----------------------------------------------------------------
# AST 분석 → 엔티티 수집
# ----------------------------------------------------------------
def collect(files, clang_args, debug_mode):
    idx = cindex.Index.create()

    structs, unions, enums = {}, {}, {}
    vars_, funcs, uses = {}, {}, []
    type_dependencies = []

    roots = {str(pathlib.Path(f).resolve().parent) for f in files}

    gen_anon_struct_id = itertools.count(1)
    gen_anon_union_id = itertools.count(1)
    gen_anon_enum_id = itertools.count(1)

    named_types_map = {}

    def in_proj(cur):
        if not cur.location or not cur.location.file:
            return False
        path = cur.location.file.name
        normalized_path = str(pathlib.Path(path).resolve())

        is_in = False
        for r in roots:
            normalized_root = str(pathlib.Path(r).resolve())
            if normalized_path.startswith(normalized_root):
                is_in = True
                break
        return is_in

    # 새 헬퍼 함수: 타입을 재귀적으로 파고들어 실제 선언을 찾습니다.
    def resolve_underlying_type(clang_type, current_in_proj, named_types_map, depth=0):
        if depth > 10: # 무한 재귀 방지
            if debug_mode:
                print(f"[DEBUG-Resolve] Max depth reached for type: {clang_type.spelling}")
            return None

        # 1. 포인터 타입인 경우, 가리키는 타입으로 넘어갑니다.
        if clang_type.kind == cindex.TypeKind.POINTER:
            if debug_mode:
                print(f"[DEBUG-Resolve] Depth {depth}: Following pointer from {clang_type.spelling} to pointee {clang_type.get_pointee().spelling}")
            return resolve_underlying_type(clang_type.get_pointee(), current_in_proj, named_types_map, depth + 1)

        # 2. 배열 타입인 경우, 요소 타입으로 넘어갑니다.
        if clang_type.kind in (cindex.TypeKind.CONSTANTARRAY, cindex.TypeKind.INCOMPLETEARRAY, cindex.TypeKind.VARIABLEARRAY):
            if debug_mode:
                print(f"[DEBUG-Resolve] Depth {depth}: Following array from {clang_type.spelling} to element {clang_type.element_type.spelling}")
            return resolve_underlying_type(clang_type.element_type, current_in_proj, named_types_map, depth + 1)

        # 3. 타입의 정규화된 선언을 가져옵니다.
        decl_cursor = clang_type.get_canonical().get_declaration()

        if not decl_cursor:
            return None # 의미 있는 선언이 없으면 종료

        # 4. TYPEDEF_DECL 인 경우, Underlying Type을 다시 resolve 합니다.
        if decl_cursor.kind == cindex.CursorKind.TYPEDEF_DECL:
            if debug_mode:
                print(f"[DEBUG-Resolve] Depth {depth}: Following TYPEDEF_DECL {decl_cursor.spelling} to its underlying type {decl_cursor.underlying_type.spelling}")
            return resolve_underlying_type(decl_cursor.underlying_type, current_in_proj, named_types_map, depth + 1)

        # 5. STRUCT/UNION/ENUM_DECL 이고 프로젝트 내부에 있는 경우, 해당 이름을 반환합니다.
        if decl_cursor.kind in (cindex.CursorKind.STRUCT_DECL, cindex.CursorKind.UNION_DECL, cindex.CursorKind.ENUM_DECL):
            if decl_cursor.hash in named_types_map and current_in_proj(decl_cursor): # named_types_map에 있는지 & 프로젝트에 있는지 확인
                if debug_mode:
                    print(f"[DEBUG-Resolve] Depth {depth}: Found named type {named_types_map[decl_cursor.hash]} for {clang_type.spelling}")
                return named_types_map[decl_cursor.hash]

        # 위의 어떤 조건에도 해당하지 않으면, 기본 타입이거나 의존성 추적 대상이 아님
        return None

    for src in files:
        if debug_mode:
            print(f"\n[DEBUG-Collect] Attempting to parse: {src}")
        try:
            tu = idx.parse(src, args=clang_args)
            if not tu:
                print(f"⚠️ Warning: Failed to create TranslationUnit for {src}. Skipping.")
                continue

            for diag in tu.diagnostics:
                severity_map = {
                    cindex.Diagnostic.Ignored: "Ignored",
                    cindex.Diagnostic.Note: "Note",
                    cindex.Diagnostic.Warning: "Warning",
                    cindex.Diagnostic.Error: "Error",
                    cindex.Diagnostic.Fatal: "Fatal"
                }
                print(f"[{severity_map.get(diag.severity, 'Unknown')}] in {src}: {diag.location} {diag.spelling}")

            if not tu.cursor:
                continue

            for cur in tu.cursor.walk_preorder():
                if in_proj(cur):
                    # VAR_DECL (전역, 정적, 지역 변수) 및 PARM_DECL (함수 매개변수) 처리
                    if cur.kind == cindex.CursorKind.VAR_DECL or cur.kind == cindex.CursorKind.PARM_DECL:
                        # 변수 이름을 딕셔너리 키로 사용
                        var_key = cur.spelling
                        if var_key not in vars_:
                            is_static = False
                            # VAR_DECL만 storage_class 속성을 가짐 (정적 변수 판단용)
                            if cur.kind == cindex.CursorKind.VAR_DECL:
                                if cur.storage_class.name == "STATIC":
                                    is_static = True
                            
                            vars_[var_key] = {
                                "id": newid(),
                                "type": cur.type.spelling,
                                "static": is_static
                            }
                            if debug_mode:
                                print(f"[DEBUG-Collect] Found Var/Parm: {var_key} (Type: {cur.type.spelling}, Static: {is_static})")

                        # 수집된 모든 변수에 대해 타입 의존성 추가
                        resolved_type_name = resolve_underlying_type(cur.type, in_proj, named_types_map)
                        if resolved_type_name:
                            type_dependencies.append((var_key, resolved_type_name))
                            if debug_mode:
                                print(f"[DEBUG-Dependency] Added Var/Parm -> Type: {var_key} -> {resolved_type_name}")

                    elif cur.kind == cindex.CursorKind.FUNCTION_DECL and cur.is_definition():
                        func_name = cur.spelling
                        if func_name not in funcs:
                            funcs[func_name] = {"id": newid()}
                            if debug_mode:
                                print(f"[DEBUG-Collect] Found Function: {func_name}")

                        # 함수 내에서 참조되는 변수/매개변수 사용 관계 수집
                        for n in cur.walk_preorder():
                            if n.kind == cindex.CursorKind.DECL_REF_EXPR and n.referenced:
                                # 참조된 대상이 VAR_DECL 또는 PARM_DECL인 경우
                                if n.referenced.kind == cindex.CursorKind.VAR_DECL or \
                                   n.referenced.kind == cindex.CursorKind.PARM_DECL:
                                    referenced_var_name = n.referenced.spelling
                                    # 해당 변수/매개변수가 vars_에 수집되었는지 확인
                                    if referenced_var_name in vars_:
                                        uses.append((func_name, referenced_var_name))
                                        if debug_mode:
                                            print(f"[DEBUG-Usage] Added Func -> Var/Parm: {func_name} -> {referenced_var_name}")
                    
                    # STRUCT/UNION_DECL 및 ENUM_DECL 블록은 기존과 동일
                    elif cur.kind in (cindex.CursorKind.STRUCT_DECL, cindex.CursorKind.UNION_DECL) and cur.is_definition():
                        tag = "struct" if cur.kind == cindex.CursorKind.STRUCT_DECL else "union"

                        if not cur.spelling:
                            anon_name = f"<anon_{tag}_{next(gen_anon_struct_id) if tag=='struct' else next(gen_anon_union_id)}>"
                        else:
                            anon_name = cur.spelling

                        key = f"{tag}:{anon_name}"
                        if key not in (structs if tag=='struct' else unions):
                            items_for_display = []
                            items_for_dependency = []

                            for field in cur.get_children():
                                if field.kind == cindex.CursorKind.FIELD_DECL:
                                    display_text = f"{field.type.spelling} {field.spelling}"
                                    sanitized_key_for_idpos = f"{anon_name}.{display_text.replace(' ', '_').replace('=', '_').replace(';', '_').replace('(', '_').replace(')', '_').replace('*', '_')}"

                                    items_for_display.append(display_text)
                                    items_for_dependency.append((display_text, sanitized_key_for_idpos))

                                    # 필드 타입이 구조체/유니온/열거형일 경우 의존성 추가 (resolve_underlying_type 사용)
                                    resolved_field_type_name = resolve_underlying_type(field.type, in_proj, named_types_map)
                                    if resolved_field_type_name:
                                        type_dependencies.append((sanitized_key_for_idpos, resolved_field_type_name))
                                        if debug_mode:
                                            print(f"[DEBUG-Dependency] Added Field -> Type: {sanitized_key_for_idpos} -> {resolved_field_type_name}")

                            entry = {"id": newid(), "name": anon_name, "items": items_for_display, "items_for_dependency": items_for_dependency}
                            (structs if tag=='struct' else unions)[key] = entry
                            named_types_map[cur.hash] = anon_name

                    elif cur.kind == cindex.CursorKind.ENUM_DECL and cur.is_definition():
                        if not cur.spelling:
                            anon_name = f"<anon_enum_{next(gen_anon_enum_id)}>"
                        else:
                            anon_name = cur.spelling

                        key = f"enum:{anon_name}"
                        if key not in enums:
                            items_for_display = []
                            items_for_dependency = []
                            for c in cur.get_children():
                                if c.kind == cindex.CursorKind.ENUM_CONSTANT_DECL:
                                    display_text = f"{c.spelling} = {c.enum_value}"
                                    sanitized_key_for_idpos = f"{anon_name}.{c.spelling.replace(' ', '_')}"

                                    items_for_display.append(display_text)
                                    items_for_dependency.append((display_text, sanitized_key_for_idpos))

                            enums[key] = {"id": newid(), "name": anon_name, "items": items_for_display, "items_for_dependency": items_for_dependency}
                            named_types_map[cur.hash] = anon_name

        except cindex.TranslationUnitLoadError as e:
            print(f"❌ Error loading translation unit for {src}: {e}. Skipping.")
        except Exception as e:
            print(f"❌ An unexpected error occurred while processing {src}: {e}. Skipping.")

    return structs, unions, enums, vars_, funcs, uses, type_dependencies

# ----------------------------------------------------------------
# draw.io 저장
# ----------------------------------------------------------------
def save(structs, unions, enums, vars_, funcs, uses, type_dependencies, outname, debug_mode):
    mx = ET.Element("mxfile", host="app.diagrams.net")
    diagram = ET.SubElement(mx, "diagram", id="page", name="Page-1")
    model = ET.Element("mxGraphModel")
    root = ET.SubElement(model, "root")
    ET.SubElement(root, "mxCell", id="0")
    ET.SubElement(root, "mxCell", id="1", parent="0")

    # 레이아웃 변수
    W = 220
    min_H_parent_node = 50
    min_H_child_node = 30

    dx, dy = 240, 160
    item_spacing_y = 5

    # 색상 팔레트
    pal = {
        "struct": "#43B5A0", # 기존 구조체 색상 유지 (녹색 계열)
        "union": "#7EAAD4",  # 기존 유니온 색상 유지 (파란색 계열)
        "enum": "#AF7AC5",   # 기존 열거형 색상 유지 (보라색 계열)
        "global": "#FFC857", # 전역 변수
        "static": "#E9724C", # 정적 변수
        "func": "#34495E",   # 함수
        "child_node": "#FF6347" # 내부 필드/상수 노드 색상: 토마토 레드 (아주 눈에 띄게)
    }

    idpos = {}

    current_global_x, current_global_y = 0, 0
    max_row_height = 0
    col_count = 3 # 한 행에 그릴 노드 수

    all_parent_infos = []
    for ent_type, collection, label, color in [
        ("struct", structs, "struct", pal["struct"]),
        ("union", unions, "union", pal["union"]),
        ("enum", enums, "enum", pal["enum"])
    ]:
        for name, info in collection.items():
            all_parent_infos.append((info, label, color))

    all_parent_infos.sort(key=lambda x: int(x[0]["id"]))

    for parent_info, parent_label, parent_color in all_parent_infos:
        parent_id = parent_info["id"]
        parent_name = parent_info["name"]
        idpos[parent_name] = parent_id

        parent_node_x = current_global_x
        parent_node_y = current_global_y

        node_value = f"<b>{parent_label} {parent_name}</b>"
        parent_cell = ET.SubElement(root, "mxCell", id=parent_id, value=node_value, vertex="1", parent="1",
            style=f"shape=rect;rounded=0;html=1;whiteSpace=wrap;strokeColor=#333;fillColor={parent_color};fontColor=#FFF;align=center;verticalAlign=middle;autoSize=1;spacing=4;overflow=hidden;")
        ET.SubElement(parent_cell, "mxGeometry", x=str(parent_node_x), y=str(parent_node_y), width=str(W), height=str(min_H_parent_node), attrib={"as":"geometry"})

        current_child_y = parent_node_y + min_H_parent_node + item_spacing_y

        # 첫 번째 자식 노드의 이전 노드 ID (연결을 위함)
        previous_child_id = parent_id # 첫 연결은 부모 노드에서 시작

        for item_display_text, item_sanitized_key in parent_info["items_for_dependency"]:
            child_id = newid()
            idpos[item_sanitized_key] = child_id

            child_cell = ET.SubElement(root, "mxCell", id=child_id, value=item_display_text, vertex="1", parent="1",
                style=f"shape=rect;rounded=0;html=1;whiteSpace=wrap;strokeColor=#AAA;fillColor={pal['child_node']};fontColor=#FFF;align=left;verticalAlign=middle;autoSize=1;spacing=4;overflow=hidden;fontSize=10;")
            ET.SubElement(child_cell, "mxGeometry", x=str(parent_node_x), y=str(current_child_y), width=str(W), height=str(min_H_child_node), attrib={"as":"geometry"})

            # 이전 노드와 현재 자식 노드 연결
            edge_id = newid()
            ET.SubElement(root, "mxCell", id=edge_id, edge="1", parent="1",
                source=previous_child_id, target=child_id,
                style="endArrow=block;html=1;strokeColor=#000000;strokeWidth=2;")
            previous_child_id = child_id # 다음 연결을 위해 현재 자식 노드를 '이전 노드'로 설정

            current_child_y += min_H_child_node + item_spacing_y

        group_total_height = current_child_y - parent_node_y
        max_row_height = max(max_row_height, group_total_height)

        current_global_x += W + dx
        if (current_global_x // (W + dx)) >= col_count:
            current_global_x = 0
            current_global_y += max_row_height + dy
            max_row_height = 0

    if all_parent_infos:
        if current_global_x != 0:
             current_global_y += max_row_height + dy
        current_global_x = 0
        max_row_height = 0

    # 독립 노드 (변수, 함수) 레이아웃은 이전과 동일하게 유지
    for name, info in vars_.items():
        nid = info["id"]; idpos[name] = nid
        color = pal["static"] if info["static"] else pal["global"]

        node_value = f"<b>{name}</b><br>({info['type']})"

        v = ET.SubElement(root, "mxCell", id=nid, value=node_value, vertex="1", parent="1",
            style=f"shape=rect;rounded=0;html=1;whiteSpace=wrap;strokeColor=#333;fillColor={color};fontColor=#000;align=center;verticalAlign=middle;autoSize=1;spacing=4;overflow=hidden;")
        ET.SubElement(v, "mxGeometry", x=str(current_global_x), y=str(current_global_y), width=str(W), height=str(50), attrib={"as":"geometry"})

        current_global_x += W + dx
        if (current_global_x // (W + dx)) >= col_count:
            current_global_x = 0
            current_global_y += 50 + dy

    if vars_ or all_parent_infos:
        if current_global_x != 0:
            current_global_y += 50 + dy
        current_global_x = 0

    for fn, info in funcs.items():
        nid = info["id"]; idpos[fn] = nid

        node_value = f"<b>{fn}</b>"

        c = ET.SubElement(root, "mxCell", id=nid, value=node_value, vertex="1", parent="1",
            style=f"shape=rect;rounded=0;html=1;whiteSpace=wrap;strokeColor=#333;fillColor={pal['func']};fontColor=#FFF;align=center;verticalAlign=middle;autoSize=1;spacing=4;overflow=hidden;")
        ET.SubElement(c, "mxGeometry", x=str(current_global_x), y=str(current_global_y), width=str(W), height=str(50), attrib={"as":"geometry"})

        current_global_x += W + dx
        if (current_global_x // (W + dx)) >= col_count:
            current_global_x = 0
            current_global_y += 50 + dy

    eid = next(gen_id)

    # 함수 -> 변수 사용 관계
    for fn, vname in uses:
        if fn in idpos and vname in idpos:
            e = ET.SubElement(root, "mxCell", id=str(eid), edge="1", parent="1", source=idpos[fn], target=idpos[vname],
                style="endArrow=open;html=1;strokeColor=#2980B9;strokeWidth=2;")
            ET.SubElement(e, "mxGeometry", relative="1", attrib={"as":"geometry"})
            eid += 1

    # 타입 의존성 관계 (변수/필드 -> 구조체/유니온/열거형) - 점선 화살표
    for source_name, target_type_name in type_dependencies:
        if source_name in idpos and target_type_name in idpos:
            e = ET.SubElement(root, "mxCell", id=str(eid), edge="1", parent="1",
                source=idpos[source_name], target=idpos[target_type_name],
                style="endArrow=block;html=1;strokeColor=#4A235A;strokeWidth=2;dashed=1;")
            ET.SubElement(e, "mxGeometry", relative="1", attrib={"as":"geometry"})
            eid += 1


    xml = ET.tostring(model, encoding="utf-8")
    raw = zlib.compress(xml, 9)[2:-4]
    diagram.text = base64.b64encode(raw).decode(); diagram.set("compressed", "true")
    ET.ElementTree(mx).write(outname, encoding="utf-8", xml_declaration=True)
    if debug_mode:
        print(f"[DEBUG-Save] XML written to {outname} (size: {pathlib.Path(outname).stat().st_size} bytes)")
    print("✅ saved:", outname)

if __name__ == "__main__":
    ap = argparse.ArgumentParser(
        description="C/C++ 소스 파일을 분석하여 구조체, 유니온, 열거형, 변수, 함수 및 그 사용 관계를 시각화하는 .drawio 다이어그램을 생성합니다."
    )
    ap.add_argument("paths", nargs='+',
                    help="분석할 파일 또는 디렉토리 경로 (예: ., ./src, my_file.c)")
    ap.add_argument("-I", dest="incs", action="append", default=[], metavar="DIR",
                    help="include 디렉토리 경로 (Clang에 전달)")
    ap.add_argument("-D", dest="defs", action="append", default=[], metavar="MACRO",
                    help="전처리기 매크로 정의 (예: MY_MACRO=1)")
    ap.add_argument("--std", default="c11",
                    help="C/C++ 표준 (예: c99, c11, c++11, c++17 등)")
    ap.add_argument("-o", "--output", default="var_diagram.drawio",
                    help="생성될 .drawio 파일 이름")
    ap.add_argument("--debug", action="store_true",
                    help="디버그 정보 출력")
    ap.add_argument("--libclang-path", dest="libclang_path_arg",
                    help="libclang.so 파일의 전체 경로 (환경 변수 LIBCLANG_PATH보다 우선)")
    args = ap.parse_args()

    if args.libclang_path_arg:
        cindex.Config.set_library_file(args.libclang_path_arg)
        print(f"Using libclang from: {args.libclang_path_arg}")
    elif os.environ.get("LIBCLANG_PATH"):
        print(f"Using libclang from LIBCLANG_PATH: {os.environ.get('LIBCLANG_PATH')}")
    else:
        print(f"Using default libclang path: {cindex.Config.library_file}")

    files = list(gather(args.paths, args.debug))

    if not files:
        print("❌ No source files found for analysis. Please check your 'paths' argument.")
        exit(1)

    clang_lang_arg = "-x"
    if args.std.startswith("c++"):
        clang_lang_val = "c++"
    else:
        clang_lang_val = "c"

    cargs = [clang_lang_arg, clang_lang_val, f"-std={args.std}"]
    for I in args.incs:
        cargs.extend(("-I", I))
    for d in args.defs:
        cargs.append(f"-D{d}")

    print(f"\nAnalyzing {len(files)} files with Clang arguments: {' '.join(cargs)}")
    s, u, e, v, f, us, td = collect(files, cargs, args.debug)

    if args.debug:
        print(f"\n[DEBUG-Summary] Final counts: struct={len(s)} union={len(u)} enum={len(e)} var={len(v)} func={len(f)}")
        print(f"[DEBUG-Summary] Relationships: uses={len(us)} type_dependencies={len(td)}")

    save(s, u, e, v, f, us, td, args.output, args.debug)