#include <string>
#include "PlaneEnemy.hpp"

PlaneEnemy::PlaneEnemy(int x, int y)
    : Enemy("play/enemy-2.png", x, y, 8, 80, 7, 7) { // 生命8, 金錢80, 速度7, 半徑7，可自行調整
} 