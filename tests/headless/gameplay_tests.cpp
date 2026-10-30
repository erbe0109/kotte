#include "kotte/application.hpp"

#include <raylib.h>
#include <array>
#include <cmath>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <string_view>

namespace headless {
    void reset_keys();
    extern std::array<bool, 512> pressed;
    extern std::array<bool, 512> held;
}

namespace kotte {
    struct GameTestAccess final {
        static void frame(Application& game, float dt = 0.016f, int pressed = -1, int held = -1){
            headless::reset_keys();
            if(pressed >= 0){ headless::pressed[static_cast<std::size_t>(pressed)] = true; }
            if(held >= 0){ headless::held[static_cast<std::size_t>(held)] = true; }
            game.run_frame(dt);
            headless::reset_keys();
        }
        static void move(Application& game, EntityHandle handle, Vector2 target){
            Entity& entity = game.entity(handle);
            const Rectangle previous = game.entity_world_bounds(entity);
            entity.world_position = target;
            game.spatial_grid_.update(handle, previous, game.entity_world_bounds(entity));
        }
        static bool playing(const Application& g){return g.round_state_ == RoundState::playing;}
        static bool won(const Application& g){return g.round_state_ == RoundState::won;}
        static bool lost(const Application& g){return g.round_state_ == RoundState::lost;}
        static EntityHandle player(const Application& g){return g.player_handle_;}
        static Vector2 position(const Application& g, EntityHandle handle){return g.entity(handle).world_position;}
        static std::size_t bombs(const Application& g){return g.active_bomb_handles_.size();}
        static std::size_t enemies(const Application& g){return g.enemy_handles_.size();}
        static std::uint8_t range(const Application& g){return g.player_blast_range_;}
        static EntityHandle bomb(const Application& g, std::size_t index){return g.active_bomb_handles_.at(index);}
        static EntityHandle enemy(const Application& g, std::size_t index){return g.enemy_handles_.at(index);}
        static EntityHandle pickup(const Application& g){
            const auto q = g.spatial_grid_.query({120.0f, 80.0f, 40.0f, 40.0f});
            for(const auto handle : q.entity_handles){
                const Entity* e = g.entities_.try_get(handle);
                if(e != nullptr && e->kind == EntityKind::range_pickup){return handle;}
            }
            return {};
        }
        static bool alive(const Application& g, EntityHandle handle){return g.entities_.contains(handle);}
        static bool movement_blocked(Application& g, EntityHandle mover, Vector2 target){
            return g.entity_movement_is_blocked(mover, target);
        }
        static void add_pickup(Application& g, Vector2 position){
            Entity pickup{};
            pickup.kind = EntityKind::range_pickup;
            pickup.world_position = position;
            const auto handle = g.entities_.create(pickup);
            g.spatial_grid_.insert(handle, g.entity_world_bounds(pickup));
        }
        static bool escape(const Application& g, EntityHandle handle){
            return g.entity(handle).bomb_state->owner_escape_allowed;
        }
        static std::uint8_t bomb_range(const Application& g, EntityHandle handle){
            return g.entity(handle).bomb_state->blast_range;
        }
        static void detonate(Application& g, EntityHandle handle){
            g.request_bomb_detonation(handle);
            g.resolve_gameplay_facts();
            g.apply_structural_mutations();
            g.finish_round();
        }
        static std::size_t effects(const Application& g){return g.blast_effects_.size();}
        static void kill_all_enemies(Application& g){
            for(const EntityHandle handle : g.enemy_handles_){
                g.destroy_entity_requests_.push_back({handle});
            }
            g.apply_structural_mutations();
            g.finish_round();
        }
    };
}

using Game = kotte::Application;
using Test = kotte::GameTestAccess;

void require(bool success, std::string_view message){
    if(!success){throw std::runtime_error{std::string{message}};}
}

void test_owner_escape_and_pickup(){
    Game g{1280, 720, "test", 0x4b4f545445ULL};
    require(Test::playing(g) && Test::enemies(g) == 3, "initial state/enemy quota");
    const auto pickup = Test::pickup(g);
    require(Test::alive(g, pickup) && Test::range(g) == 3, "pickup exists / range 3");
    Test::frame(g, 0.016f);
    require(Test::range(g) == 3, "nearby pickup not collected at starting tile");
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::bombs(g) == 1, "placed first bomb");
    const auto old = Test::bomb(g, 0);
    require(Test::movement_blocked(g, Test::enemy(g, 0), {100.0f, 100.0f}),
        "other actors cannot use bomb owner escape exception");
    require(Test::bomb_range(g, old) == 3, "range captured at bomb creation");
    Test::frame(g, 0.03f, -1, KEY_D);
    require(Test::position(g, Test::player(g)).x > 100.0f
        && Test::escape(g, old),
        "owner can move while still overlapping a newly placed bomb");
    Test::frame(g, 0.07f, -1, KEY_D);
    require(!Test::escape(g, old), "owner escape ends when fully outside bomb");
    require(Test::range(g) == 4 && !Test::alive(g, pickup), "pickup collected once and destroyed");
    const float x = Test::position(g, Test::player(g)).x;
    Test::frame(g, 0.05f, -1, KEY_A);
    require(std::abs(Test::position(g, Test::player(g)).x - x) < 0.01f,
        "solid bomb blocks reentry after owner leaves");
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::bombs(g) == 2, "second bomb can be placed after leaving the first");
    const auto newer = Test::bomb(g, 1);
    require(Test::bomb_range(g, newer) == 4, "new bomb captures upgraded range");
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::bombs(g) == 2, "same tile cannot hold two bombs");
    Test::frame(g, 0.20f, -1, KEY_D);
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::bombs(g) == 3, "third bomb within capacity");
    Test::move(g, Test::player(g), {220.0f, 100.0f});
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::bombs(g) == 3, "fourth bomb denied at capacity");

    Test::move(g, Test::player(g), {60.0f, 300.0f});
    Test::frame(g, 0.016f, KEY_E);
    require(Test::playing(g) && Test::bombs(g) == 0,
        "remote command removes all owned bombs");
    require(!Test::movement_blocked(g, Test::player(g), {100.0f, 100.0f}),
        "bomb stops blocking the tile when destroyed");
    Test::frame(g, 0.016f, KEY_E);
    require(Test::bombs(g) == 0, "remote command with no bombs is safe");
    Test::frame(g, 0.50f);
    Test::move(g, Test::player(g), {100.0f, 100.0f});
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::bombs(g) == 1, "capacity restored after remote explosions");
}

void test_pickup_maximum(){
    Game g{1280, 720, "test", 0x4b4f545445ULL};
    Test::move(g, Test::player(g), {140.0f, 100.0f});
    Test::frame(g, 0.0f);
    require(Test::range(g) == 4, "first pickup increases range to four");
    for(int i = 0; i < 6; ++i){
        Test::add_pickup(g, {140.0f, 100.0f});
        Test::frame(g, 0.0f);
    }
    require(Test::range(g) == 6, "range capped at six despite extra pickups");
    Test::frame(g, 0.0f);
    require(Test::range(g) == 6, "revisiting collected pickup gives no benefit");
}

void test_blast_and_restart(){
    Game g{1280, 720, "test", 0x4b4f545445ULL};
    const auto original_player = Test::player(g);
    Test::frame(g, 0.016f, KEY_SPACE);
    const auto old_bomb = Test::bomb(g, 0);
    Test::detonate(g, old_bomb);
    require(Test::lost(g) && Test::bombs(g) == 0,
        "contact with blast causes loss and removes bomb");
    require(Test::effects(g) == 1, "blast effect remains visible after loss");
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::lost(g) && Test::bombs(g) == 0,
        "gameplay stays frozen after losing");
    Test::frame(g, 0.016f, KEY_R);
    require(Test::playing(g) && Test::range(g) == 3
        && Test::enemies(g) == 3 && Test::bombs(g) == 0 && Test::effects(g) == 0,
        "restart clears active effects and restores all game state");
    require(!Test::alive(g, old_bomb) && !Test::alive(g, original_player),
        "restart invalidates old bomb and player handles");
    const auto next_player = Test::player(g);
    Test::frame(g, 0.016f, KEY_SPACE);
    require(Test::bombs(g) == 1, "gameplay resumes after restart");
    Test::move(g, next_player, {420.0f, 140.0f});
    Test::frame(g, 0.0f);
    require(Test::lost(g), "exact contact with enemy causes loss");
    Test::frame(g, 0.016f, KEY_ENTER);
    require(Test::playing(g) && !Test::alive(g, next_player),
        "second restart also invalidates previous generation");
}

void test_enemy_near_miss(){
    Game g{1280, 720, "test", 0x4b4f545445ULL};
    const Vector2 enemy = Test::position(g, Test::enemy(g, 0));
    Test::move(g, Test::player(g), {enemy.x, enemy.y + 32.0f});
    Test::frame(g, 0.0f);
    require(Test::playing(g), "near enemy in broad-phase cell is not exact hit");
    Test::move(g, Test::player(g), enemy);
    Test::frame(g, 0.0f);
    require(Test::lost(g), "exact enemy overlap is fatal");
    Test::frame(g, 0.50f);
    require(Test::lost(g), "loss remains stable across later frames");
}

void test_chain_win_and_priority(){
    Game g{1280, 720, "test", 0x4b4f545445ULL};
    Test::frame(g, 0.016f, KEY_SPACE);
    const auto first = Test::bomb(g, 0);
    Test::move(g, Test::player(g), {180.0f, 100.0f});
    Test::frame(g, 0.016f, KEY_SPACE);
    const auto second = Test::bomb(g, 1);
    require(Test::bombs(g) == 2, "two bombs in same blast ray");
    Test::move(g, Test::player(g), {60.0f, 300.0f});
    Test::detonate(g, first);
    require(!Test::alive(g, first) && !Test::alive(g, second)
        && Test::bombs(g) == 0 && Test::effects(g) == 2,
        "first bomb chain-detonates second exactly once");
    Test::frame(g, 3.0f);
    require(Test::playing(g) && Test::bombs(g) == 0,
        "chain-detonated bomb cannot explode again after fuse");

    Game win{1280, 720, "test", 0x4b4f545445ULL};
    Test::frame(win, 0.016f, KEY_SPACE);
    const auto explosive = Test::bomb(win, 0);
    Test::move(win, Test::player(win), {60.0f, 300.0f});
    Test::move(win, Test::enemy(win, 0), {140.0f, 100.0f});
    Test::move(win, Test::enemy(win, 1), {100.0f, 140.0f});
    Test::move(win, Test::enemy(win, 2), {180.0f, 100.0f});
    Test::detonate(win, explosive);
    require(Test::won(win) && Test::enemies(win) == 0,
        "defeating the last three enemies by blast wins the round");
    require(Test::effects(win) == 1, "win can happen with a blast still active");
    Test::frame(win, 0.016f, KEY_R);
    require(Test::playing(win) && Test::enemies(win) == 3 && Test::effects(win) == 0,
        "winning restart removes old blast and restores objective");

    Game tie{1280, 720, "test", 0x4b4f545445ULL};
    Test::frame(tie, 0.016f, KEY_SPACE);
    const auto tie_bomb = Test::bomb(tie, 0);
    Test::move(tie, Test::enemy(tie, 0), {140.0f, 100.0f});
    Test::move(tie, Test::enemy(tie, 1), {100.0f, 140.0f});
    Test::move(tie, Test::enemy(tie, 2), {180.0f, 100.0f});
    Test::detonate(tie, tie_bomb);
    require(Test::lost(tie) && Test::enemies(tie) == 0,
        "player death takes precedence over last enemy killed");
}

int main(){
    try{
        test_owner_escape_and_pickup();
        test_pickup_maximum();
        test_blast_and_restart();
        test_enemy_near_miss();
        test_chain_win_and_priority();
        std::puts("Gameplay headless tests passed: 5 scenarios.");
    } catch(const std::exception& failure){
        std::fprintf(stderr, "Gameplay tests FAILED: %s\n", failure.what());
        return 1;
    }
    return 0;
}
