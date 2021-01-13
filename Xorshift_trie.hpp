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
int create_seed(int node, uint8_t c)const{
    return (node << 8) + c;
}

public:
int node_count = 0; 

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
        uint64_t new_node = shift.get_nextnode(create_seed(node, c));
        if(new_node == -1){
            std::cout << str << "  " << c << std::endl;
            return false;
        }
        node = new_node;
    }
    return shift.get_nextnode(create_seed(node,kLeafChar)) != -1;
}

void xor_try(const std::string& str) {
    uint64_t node = 0;
   // std::cout << "-----key_word-------" << str << std::endl;
    for (uint8_t c : str) {
        //std::cout << "  " << c << std::endl;
        uint64_t t = shift.get_nextnode(create_seed(node,c));
        if(t != -1){    
            node = t;
            //std::cout << "--common--" << "\n";
        }
        else{
            node = shift.set(node,c);
            //std::cout << "--new_node--" << "\n";
            node = shift.get_nextnode(create_seed(node,c));//衝突回数の分修正する
            node_count++;
        }
    }
    //終端文字格納
    shift.set(node,kLeafChar);
    node_count++;
    //shift.display();
    //std::cout << "node : " << node_count << std::endl;
}


};

}

#endif //XORSHIFT__XORSHIFT_HPP_
