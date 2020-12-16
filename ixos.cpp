#include <iostream>
#include<vector>
#include<algorithm>
#include <math.h>
#include <sstream>
#include <cmath>
#include <limits>
#include <cassert>

static constexpr uint64_t default_size = 1ull << 24;//(ここは24)
int B_[3] = {13,-7,5};//ビットシフトパターン(32ビット)
int B_1[3] = {5,-7,13};//ビットシフトパターン(32ビット)

int bit_len(uint32_t x) {
    assert(x > 0);
    int n = 0;
    while ((1ull<<n) <= x-1)
        ++n;
    return n;
}
int k = bit_len(default_size) + 8;//mask値の決定のため、P, C 拡張時にインクリメント

uint64_t xos(uint64_t x){//前&x
    uint64_t maskXos_ = 1ull << k;
	for(int b: B_){//ビットシフトパターン{a,b}なら先にa次にbの２回ループ
        //std::cout << "shift patern , seed" << b << "," << x << "\n";
		if(b >= 0){
			x = (x ^ (x << b)) % maskXos_;
		}
		else if(b < 0){
			x = (x ^ (x >> -b)) % maskXos_;
        }
		// std::cout << "xos b" << b << "\n";
		// std::cout << "output :" << x << "\n";
	}
	return x;
}

int loop_time(int k, int b){
    assert(k > 0 and b > 0);
    return 1 << bit_len((k-1) / b + 1);
}

uint64_t ixos(uint64_t x){//前シード値から出力
    uint64_t maskXos_ = 1ull << k;
	//xorの回数は2^ceil(log2(k/b))
	for(int b: B_1){
        int n = loop_time(k, abs(b)) - 1;

		// std::cout << "roop time" << n << "\n";
		// std::cout << "ixos b" << b << "\n";
		for(int i = 0;i < n;i++){
			if(b >= 0){
				x = (x ^ (x << b)) % maskXos_;
			}
			else if(b < 0){
				x = (x ^ (x >> -b)) % maskXos_;
			}
		}
		// std::cout << "seed" << x << "\n";
	}
	return x;
} 

int main(){
    // uint64_t seed = 98;
    int seed_width = 12;
    int ng_cnt = 0;
    for (uint64_t seed = 0; seed < 1<<seed_width; seed++) {
        uint64_t x = xos(seed);//シード値→出力値
        // std::cout << "-----------------------"<< "\n";
        uint64_t s1 = ixos(x);//出力値→シード値
        bool ok = s1 == seed;
        std::cout << seed << " -> " << x << " -> " << s1 << " " << (ok ? "OK" : "Failed") << std::endl;
        ng_cnt += !ok;
    }
    std::cout << "Failed count: " << ng_cnt << std::endl;
	int a = bit_len(4);
    return 0;
}


 
