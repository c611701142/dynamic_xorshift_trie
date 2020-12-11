#ifndef XORSHIFT__xorshift_trie_HPP_
#define XORSHIFT__xorshift_trie_HPP_
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert>

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

int common_word = 0; 
public:
void xor_try(const std::string& str) {
    int node = 0;
    int collision = 0;
    int parity = 0;
    std::cout << "-----key_word-------" << str << std::endl;
    for (uint8_t c : str) {
        //std::cout << "-----trans_word----" << c << std::endl;
        uint64_t x = create_seed(node,c);//シード値
        uint64_t x1 = shift.xos(x);
        parity = shift.get_parity(x);
        collision = shift.get_collision(x);
        int t = shift.get_nextnode(x);
        int parity2 = x1 % 256;//本来のパリティ値
        //std::cout << "-----try start----" << std::endl;
        //std::cout << "x : " << x1 << std::endl;
        //std::cout << "seed : " << x << std::endl;
        //std::cout << "P : " << parity << std::endl;
        //std::cout << "c: " << collision << std::endl;
        //std::cout << "t: " << t << std::endl;
        //std::cout << "s : t " << node << " --> " << t << std::endl;
        //std::cout << "-----try end----" << std::endl;
        //接頭辞共有
        //使用要素かつパリティ値が一致 (遷移先候補のみがかぶる可能性があるので)
        //if(parity != -1 && collision != -1){
        if(shift.exists[t] && parity == parity2){    
            node = t;
            //std::cout << "--common--" << "\n";
            common_word++;
        }
        else{
            //std::cout << "---new_node---" << std::endl;
            shift.set(x);
            node = t;
        }
    }
    //終端文字格納
    uint64_t x = (node << 8) + kLeafChar;
    shift.set(x);
    //std::cout << "---insert_#---" << std::endl;
    //std::cout << common_word << std::endl;
    //shift.display();
}


};

}

#endif //XORSHIFT__XORSHIFT_HPP_
