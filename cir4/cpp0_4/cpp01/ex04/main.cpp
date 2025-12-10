/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Jaeholee <makest@naver.com>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/15 18:11:02 by Jaeholee          #+#    #+#             */
/*   Updated: 2025/10/15 18:11:05 by Jaeholee         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <iostream>
#include <string>

enum err_num {
	eOK, eARG, eEMPTY, eOPEN, eSAVE, eREAD
};

err_num e_check( err_num err ) {
	const static std::string words[] = {
		"replace done!\n",
		"Only three args are must needed, filename old_str new_str!\n",
		"old_str can't empty!\n",
		"Check the filename or file!\n",
		"Can't make or open filename.replace!\n",
		"Something wrong while read a file!\n"
	};
	std::cout << words[err];
	return err;
}

int main(int argc, char* argv[]) {
	if (argc != 4) { return e_check(eARG); }
	
	// 받은인자 string으로
	std::string filename(argv[1]), old_str(argv[2]), new_str(argv[3]);
	if (old_str.empty()) { return e_check(eEMPTY); }
	
	// file 열기
	std::ifstream op_file(filename.c_str());
	if (!op_file.is_open()) { return e_check(eOPEN); }
	
	// .replace 추가 및 save file 열기
	std::string savename(filename.append(".replace"));
	std::ofstream save_file(savename.c_str());
	if (!save_file.is_open()) { return e_check(eSAVE); }
	
	// 변경필요 없다면 버퍼 통째로 복사
	if (old_str == new_str) { save_file << op_file.rdbuf(); return e_check(eOK); }

	std::string get_data, replace_line;
	std::size_t pos, find_pos;
	while (std::getline(op_file, get_data)) {
		pos = 0;
		while (true) {
			find_pos = get_data.find(old_str, pos);
			if (find_pos == std::string::npos) {
				replace_line.append(get_data, pos, find_pos);
				break;
			}
			replace_line.append(get_data, pos, find_pos - pos);
			replace_line.append(new_str);
			pos = find_pos + old_str.size();
		}
		if (!op_file.eof()) replace_line.append("\n");
	}
	//if (!op_file.eof()) { return e_check(eREAD);}
	save_file << replace_line;
	return e_check(eOK);
}





















