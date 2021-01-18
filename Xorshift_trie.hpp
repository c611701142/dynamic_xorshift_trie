#ifndef XORSHIFT__xorshift_trie_HPP_
#define XORSHIFT__xorshift_trie_HPP_
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <string>


#include "Xorshift.hpp"

namespace kuroda {

class xorshift_trie{
private:
kuroda::Xorshift shift;
static constexpr uint8_t kLeafChar = 0;

private:
//static constexpr uint8_t kLeafChar = '0';

public:
int node_count = 0; 
int common = 0;
int c_max = 0;
int mask;
int re_take;

bool contains_all(const std::vector<std::string>& str_list, int size) {

    for(int i=0; i < size; i++) {
        bool is_check = contains(str_list[i]);
        if(is_check == false){
            std::cout << "failed..." << std::endl;
            //exit(0);//プログラム異常終了
            return false;
        }
    }
    return true;
}

bool contains(const std::string& str)const{//文字列strが辞書にあるかどうか検索
    uint64_t node = 0; // root
    for (uint8_t c : str) {
        int new_node = shift.get_nextnode(node, c);
        if(new_node == shift.invalid){
            std::cout << "***************" << std::endl;
            std::cout << node << "  " << c << std::endl;
            return false;
        }
        node = new_node;
    }
    return shift.get_nextnode(node,kLeafChar) != shift.invalid;
}

int word = 0;
void xor_try(const std::string& str) {
    int node = 0;
    word++;
    if(word % 100000 == 0){
        std::cout << word << "  " << shift.k <<  std::endl;
    }
    for (uint8_t c : str) {
        //std::cout << "  " << c << std::endl;
        int t = shift.get_nextnode(node,c);
        if(t != shift.invalid){    
            node = t;
            //std::cout << "--common--" << "\n";
            common++;
        }
        else{
            node = shift.set(node,c);
            //std::cout << "--new_node--" << "\n";
            node = shift.get_nextnode(node,c);//衝突回数の分修正する
            node_count++;
        }
    }
    //終端文字格納
    shift.set(node,kLeafChar);
    node_count++;
    //std::cout << str << " common : " << seto << std::endl;
    mask = shift.k;
    re_take = shift.replace_time;
}

void display(){
    for(uint64_t i = 1; i < shift.pc_.size();i++){
        //使用要素のみ表示
        if(shift.exists[i]){
            if(c_max < shift.pc_[i].c){
                c_max = shift.pc_[i].c;
            }
            //std::cout << i << "       ";
            //std::cout << shift.pc_[i].p << "  |  " << shift.pc_[i].c << "  " << std::endl;
            //配列番号
        }
    }
    std::cout << "collision_max" << c_max << std::endl;
    //std::cout << "mask :" << k << std::endl;
}



};

}

#endif //XORSHIFT__XORSHIFT_HPP_
