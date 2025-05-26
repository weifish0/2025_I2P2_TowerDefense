#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Bullet/MissileBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "MissileTurret.hpp"
#include "Scene/PlayScene.hpp"

const int MissileTurret::Price = 0;
MissileTurret::MissileTurret(float x, float y)
    : Turret("play/tower-base.png", "play/turret-3.png", x, y, 1000, Price, 0.5) {
    // Move center downward, since we the turret head is slightly biased upward.
    Anchor.y += 8.0f / GetBitmapHeight();
}

void MissileTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new MissileBullet(Position + normalized * 36, diff, rotation, this));
    AudioHelper::PlayAudio("gun.wav");
}
