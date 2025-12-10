#!/usr/bin/env python3
# draw.py – C 함수 호출 그래프 → draw.io(.drawio)   ★ 필터 옵션 지원 ★

# python3 draw_all.py . -I include --drop-unreferenced --drop-extern --keep-main -o call_tree.drawio

# --drop-unreferenced	다른 함수에게 한 번도 호출되지 않는 함수( in-degree 0 ) 제거
# --drop-extern		프로젝트 안에 정의가 없는(라이브러리·시스템) 함수 제거
# --keep-main			위 두 옵션으로 걸려도 main 함수만은 유지
# --include-all-defined   프로젝트 내 정의된 모든 함수는 필터와 무관하게 유지 (새로 추가됨)

# 사용법
# python3 draw.py  src/main.c  src/utils/helper.c  core/lexer/ -I include -I third_party/headers -o my_call_tree.drawio

# 부분					의미
# src/main.c src/utils/helper.c	정확히 이 두 개 파일만 분석
# core/lexer/				폴더 하나 추가 → 그 안의 *.c · *.h 전부 재귀 탐색
# -I include				헤더를 찾을 1번 경로
# -I third_party/headers		헤더를 찾을 2번 경로
# -o my_call_tree.drawio		결과 파일 이름 지정

import argparse, pathlib, sys, xml.etree.ElementTree as ET
import networkx as nx, zlib, base64
from clang import cindex

# ─────────── ①  libclang.so 경로만 본인 환경에 맞게! ───────────
cindex.Config.set_library_file(
    "/home/jaeholee/.local/lib/python3.10/site-packages/clang/native/libclang.so"
)
# ───────────────────────────────────────────────────────────────


# ─── 소스(.c/.h) 수집 ───────────────────────────────────────────
def collect(paths):
    for p in paths:
        p = pathlib.Path(p)
        if p.is_dir():
            yield from (str(f) for f in p.rglob("*.[ch]"))
        elif p.suffix in {".c", ".h"}:
            yield str(p)


# ─── 호출 그래프 + “프로젝트 정의 함수” 집합 ─────────────────────
def build(files, incs):
    idx, G = cindex.Index.create(), nx.DiGraph()
    
    # ★ 이 부분이 수정되었습니다. ★
    opts = ["-x", "c"]
    for i_path in incs:
        if i_path == "/usr/include": # /usr/include는 -isystem으로 처리
            opts.append("-isystem")
            opts.append(i_path)
        else: # 그 외의 -I 경로는 그대로 -I로 처리
            opts.append(f"-I{i_path}")
    
    # ★ 여기에 추가: clang -v에서 찾은 정확한 GCC 시스템 헤더 경로 ★
    # 사용자님의 출력에서 "Selected GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/12" 이므로
    # 해당 경로의 include 디렉토리를 추가합니다.
    opts.append("-isystem")
    opts.append("/usr/lib/gcc/x86_64-linux-gnu/12/include") 
    
    opts.append("-pthread") # pthread 라이브러리 심볼 처리용
    defined = set()

    for src in files:
        tu = idx.parse(src, args=opts)
        # Clang 파싱 중 진단 메시지 출력 (오류/경고만)
        if tu.diagnostics:
            for d in tu.diagnostics:
                if d.severity >= cindex.Diagnostic.Warning:
                    print(f"⚠️ Clang 진단 [{src}]: {d.location}: {d.spelling}")

        for fn in tu.cursor.get_children():
            # FUNCTION_DECL 이면서 정의(definition)를 포함하는 경우
            if fn.kind.name == "FUNCTION_DECL" and fn.is_definition():
                caller = fn.spelling
                defined.add(caller) # 프로젝트 내 정의된 함수로 추가
                
                # 함수 내부를 순회하며 CALL_EXPR 찾기
                for n in fn.walk_preorder():
                    if n.kind.name == "CALL_EXPR":
                        callee = n.displayname.split("(")[0]
                        if callee:
                            G.add_edge(caller, callee)
                            # ★ 디버그 출력 활성화: '#'을 제거했습니다. ★
                            print(f"DEBUG: '{caller}'가 '{callee}' 호출 감지")
                            
                            # 호출되는 외부 함수도 노드로 즉시 추가
                            if callee not in defined: # defined에 없으면 외부 함수일 가능성 높음
                                G.add_node(callee)
    
    # 간선만으로 생기지 않은 단독 노드 보강 (정의되었지만 호출이 감지되지 않은 함수 포함)
    for def_func in defined:
        if def_func not in G: 
            G.add_node(def_func)

    # 엣지를 통해 연결된 노드도 보강 (redundant하지만 안전을 위해 유지)
    for u, v in G.edges():
        G.add_node(u)
        G.add_node(v)

    return G, defined


# ─── 필터 적용 ───────────────────────────────────────────────────
def apply_filters(G, defined, drop_unref, drop_extern, keep_main, include_all_defined):
    remove = set()
    
    if drop_unref:
        remove |= {n for n in G if G.in_degree(n) == 0}
    
    if drop_extern:
        remove |= {n for n in G if n not in defined}
    
    if include_all_defined:
        remove -= defined

    if keep_main:
        remove.discard("main")
        
    G.remove_nodes_from(remove)


# ─── draw.io(.drawio) 저장 ──────────────────────────────────────
def to_drawio(G, outfile):
    mx  = ET.Element("mxfile", host="app.diagrams.net")
    dia = ET.SubElement(mx, "diagram", id="page", name="Page-1")

    model = ET.Element("mxGraphModel")
    root  = ET.SubElement(model, "root")
    ET.SubElement(root, "mxCell", id="0")
    ET.SubElement(root, "mxCell", id="1", parent="0")

    # 노드 배치
    W, H, dx, dy = 120, 40, 160, 80
    idmap = {}
    
    sorted_nodes = sorted(G.nodes()) 
    
    for i, name in enumerate(sorted_nodes):
        nid = str(i + 2); idmap[name] = nid
        v = ET.SubElement(
            root, "mxCell",
            id=nid, value=name, vertex="1", parent="1",
            style=("shape=rectangle;rounded=0;html=1;whiteSpace=wrap;"
                   "strokeColor=#3B5360;fillColor=#DBE2EF;fontColor=#000000;")
        )
        ET.SubElement(
            v, "mxGeometry",
            x=str((i % 6) * dx), y=str((i // 6) * dy),
            width=str(W), height=str(H),
            attrib={"as": "geometry"}
        )

    eid = 1000
    for u, v in G.edges():
        e = ET.SubElement(
            root, "mxCell",
            id=str(eid), edge="1", parent="1",
            source=idmap[u], target=idmap[v],
            style="endArrow=block;html=1;strokeColor=#3B5360;"
        )
        ET.SubElement(e, "mxGeometry", relative="1", attrib={"as": "geometry"})
        eid += 1

    # ── raw-deflate (+Base64)  — 한 번만 compressobj!! ──
    inner = ET.tostring(model, encoding="utf-8")
    comp  = zlib.compressobj(level=9, wbits=-15)      # raw-deflate
    raw   = comp.compress(inner) + comp.flush()
    dia.text = base64.b64encode(raw).decode() 

    ET.ElementTree(mx).write(outfile, encoding="utf-8", xml_declaration=True)
    print("✅  저장 완료 →", outfile)


# ─── main ─────────────────────────────────────────────────────
if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="C 호출 그래프 → draw.io")
    ap.add_argument("paths", nargs="+", help="소스/헤더 파일·폴더")
    ap.add_argument("-I", dest="incs", action="append", default=[],
                    help="-I include (여러 번 지정 가능)")

    ap.add_argument("--drop-unreferenced", action="store_true",
                    help="다른 함수에게 호출되지 않는 함수 제거")
    ap.add_argument("--drop-extern", action="store_true",
                    help="프로젝트 밖(정의 없는) 함수 제거")
    ap.add_argument("--keep-main", action="store_true",
                    help="main 함수는 항상 유지")
    # 새로 추가된 플래그
    ap.add_argument("--include-all-defined", action="store_true",
                    help="프로젝트 내 정의된 모든 함수는 필터와 무관하게 유지")

    ap.add_argument("-o", "--output", default="call_tree.drawio")
    args = ap.parse_args()

    files = list(collect(args.paths))
    if not files:
        sys.exit("❌ .c/.h 파일을 찾지 못했습니다.")
    print(f"✅ 분석 대상 파일: {files}") # 디버그 출력

    graph, defined = build(files, args.incs)
    print(f"✅ 프로젝트 정의 함수 (발견된 정의): {sorted(list(defined))}") # 디버그 출력

    apply_filters(graph, defined,
                  args.drop_unreferenced, args.drop_extern, args.keep_main,
                  args.include_all_defined)
    
    print(f"✅ 최종 그래프 노드(함수) 수: {graph.number_of_nodes()}") # 디버그 출력
    print(f"✅ 최종 그래프 엣지(호출 관계) 수: {graph.number_of_edges()}") # 디버그 출력

    to_drawio(graph, args.output)