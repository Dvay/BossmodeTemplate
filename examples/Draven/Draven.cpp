// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "pch.h"
#include <map>
#include <set>
#include <queue>
#include <iomanip>

using namespace sdk;

static std::string championName = "Draven";
static std::string championVersion = "0.1.1"; 
 
CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreDraven");
PLUGIN_TYPE(PluginTypes::CHAMPION);

static std::vector<MissileInstance*> all_missiles;

std::chrono::system_clock::time_point time1;
std::map<int, MissileInstance* > q_reticles;
std::shared_ptr<BuffInstance> buff;

static bool catch_axes_combo = true;
static bool catch_axes_lasthit = true;
static bool catch_axes_laneclear = true;

static int axe_count = 0;
static int max_axes_count = 1;
static int catch_axes_mode = 0;
static bool draw_axes = false;
static bool e_gap_closer = false;
static bool ignore_evade = false;
static bool draw_catch_range = false;
static bool on_before_attack = false;
static float draven_catch_range = 600.0f;

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;

// Misc  checks
static float min_r_distance = 1200.0f;

/*
 * reticle
 */
struct reticle {
	float time = 0.0;
	MissileInstance* q_reticle;

	bool operator<(reticle const& other) const {
		return myhero->Position().distance(q_reticle->EndPos()) < myhero->Position().distance(other.q_reticle->EndPos());
	}
	bool to_delete = false;
};

std::queue<reticle> reticles;
bool is_moving = false;
std::shared_ptr<Vector3> reticle_pos;// = std::make_shared<Vector3>();

/*
 * BeforeAttack
 */
void BeforeAttack(GameObjectInstance* target, bool* process) {

	if (target == nullptr)
		return;
		
	if (settings->combo.useq && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& orbwalker->Mode() == OrbwalkModes::COMBO && axe_count < max_axes_count
		&& myhero->Q()->isReady() && myhero->inRange(target, myhero->AttackRange())) {
		myhero->Q()->Cast();
	}
	else if (settings->laneclear.useq && myhero->CurrentMana()>= myhero->Q()->GetManaCosts()
		&& orbwalker->Mode() == OrbwalkModes::LANECLEAR && axe_count < max_axes_count
		&& myhero->Q()->isReady() && myhero->inRange(target, myhero->AttackRange())) {
		myhero->Q()->Cast();
	}
	else if (settings->lasthit.useq && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& orbwalker->Mode() == OrbwalkModes::LASTHIT && axe_count < max_axes_count
		&& myhero->Q()->isReady() && myhero->inRange(target, myhero->AttackRange())) {
		myhero->Q()->Cast();
	}
	else if (settings->herass.useq && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& orbwalker->Mode() == OrbwalkModes::HERASS && axe_count < max_axes_count
		&& myhero->Q()->isReady() && myhero->inRange(target, myhero->AttackRange())) {
		myhero->Q()->Cast();
	}
	//on_before_attack = true;
}

/*
 * AfterAttack
 */
void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) { }

/*
 * SpellCast
 */
void SpellCast(GameObjectInstance* sender) {
	//	if (sender->NetworkId() == myhero->NetworkId()) {
			//	std::cout << sender->ActiveSpellCast()->SpellName()  << "  :  ";
		//}

		/*if (sender->ActiveSpellCast()->Slot() == 0 && sender->Team() != myhero->Team() && myhero->Position().distance(sender->ActiveSpellCast()->EndPosition()) < 200 && myhero->E()->isReady()) {
			auto target = targetselector->GetTargetByIndex(sender->Index());

			if (target != nullptr && myhero->inRange(target, myhero->E()->CastRange())) {
				myhero->E()->Cast(target);
			}
		}*/
}

/*
 * GapCloser
 */
void GapCloser(GameObjectInstance* sender) {

	if (orbwalker->Mode() == OrbwalkModes::NONE)
		return;

	if (e_gap_closer && myhero->CurrentMana() >= myhero->E()->GetManaCosts() && sender->Team() != myhero->Team() && myhero->E()->isReady()) {
		auto target = targetselector->GetTargetByIndex(sender->Index());

		if (target != nullptr && myhero->inRange(target.get(), myhero->E()->CastRange())) {
			myhero->E()->Cast(target);
		}
	}
}

/*
 * MissileCreate
 */
//MissileInstance* miss;
void MissileCreate(std::shared_ptr<MissileInstance> missile) {

	if (missile->Name() == "DravenSpinningReturn") {
		q_reticles.insert({ missile->NetworkID(), missile.get()});
		reticles.push({ game->GameTime() + 1200.0f,missile.get()});

		if (axe_count < max_axes_count)
			axe_count++;
		//time1 = std::chrono::system_clock::now();
	}

	//miss = missile;

	//std::cout << "Created: " << missile->Name() << ":" << missile->Slot()<< std::endl;

	/*	if (missile->Name() != "EzrealQ")
			return;

		all_missiles.push_back(missile);
		auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 2000.0f);

		if (target == nullptr)
			return;

		auto s = missile->StartPos();
		auto t = missile->EndPos();
		std::cout << predictionmanager->WillCollide(s, t, target.get(), missile->LineWidth()) << std::endl;;
		std::cout << myhero->Position().distance(target->Position()) << std::endl;
		std::cout << myhero->Q()->CastRange() << std::endl;*/
		//if (missile->Team() != myhero->Team() && myhero->Position().distance(missile->EndPos()) < 200 && myhero->E()->isReady()) {
	   //    auto target = targetselector->GetTargetByIndex(missile->SourceIndex());

		//	if (target != nullptr && myhero->inRange(target, myhero->E()->CastRange())) {
			//	myhero->E()->Cast(target);
		//	}
		//}
		//if (missile->LineWidth() > 0)
		//	std::cout << "Missile: " << missile->Team() << " : " << missile->Name() << std::endl;
}

/*
 * MissileRemove
 */
void MissileRemove(std::shared_ptr<MissileInstance> missile) {
		
	if (missile->Name() == "DravenSpinningReturn") {
		q_reticles.erase(missile->NetworkID());
		is_moving = false;
		reticles.front().to_delete = true;
		reticles.pop();
		axe_count--;
		//	std::cout << "removed: " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - time1).count()) << std::endl;
	}

	//miss = nullptr;
	//all_missiles.clear();
	//std::cout << "Missile remove: " << missile->Name() << std::endl;

//if (missile->Team() != myhero->Team() && myhero->Position().distance(missile->EndPos()) < 200 && myhero->E()->isReady()) {
//    auto target = targetselector->GetTargetByIndex(missile->SourceIndex());

 //	if (target != nullptr && myhero->inRange(target, myhero->E()->CastRange())) {
	 //	myhero->E()->Cast(target);
 //	}
 //}
 //if (missile->LineWidth() > 0)
 //	std::cout << "Missile: " << missile->Team() << " : " << missile->Name() << std::endl;
}

/*
 * BuffGain
 */
void BuffGain(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() == myhero->Index())
		return;

	//std::cout << "Buff Gain: " << buff->Name() << std::endl;
}

/*
 * BuffLose
 */
void BuffLose(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() != myhero->Index())
		return;

	//std::cout << "Buff Lose: " << buff->Name()  << std::endl;
}

/*
 * CreateObject
 */
Vector3 pos_b;
void CreateObject(std::shared_ptr<GameObjectInstance> obj) {
	//std::cout << "Created: " << obj->ObjectName() << " : " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - time1).count()) << std::endl;
	//}
}

/*
 * DeleteObject
 */
void DeleteObject(std::shared_ptr<GameObjectInstance> obj) {
	//if (obj->ObjectName().find("Draven") != std::string::npos)
	//std::cout << "Deleted: " << obj->ObjectName() << std::endl;
}

/*
 * AxeCatchingLogic
 */
void AxeCatchingLogic() {

	if(settings->enable_evading && !ignore_evade && myhero->isEvading())
	{
		return;
	}

	auto mouse_pos = game->GetMousePosition();
	bool hasBuff = myhero->hasBuff(buff, "DravenSpinningAttack");

	if (hasBuff && buff->isAlive() == 2) {


		orbwalker->SetOrbwalkingPoint(nullptr);
		is_moving = false;
		if (settings->enable_evading)
			myhero->ignoreEvade(false);
		return;
	}

	if (hasBuff && buff->isAlive())
		axe_count = buff->isAlive() + reticles.size();

	if (reticles.size() > 0) {
		auto end_pos = reticles.front().q_reticle->EndPos();
		reticle_pos = std::make_shared< Vector3>(end_pos.x, end_pos.y, end_pos.z); 
		is_moving = true;
	}

	if (!is_moving) {

		orbwalker->SetOrbwalkingPoint(nullptr);

		if (settings->enable_evading)
			myhero->ignoreEvade(false);

		return;
	}

	auto time = game->GameTime();
	auto movespeed = myhero->aiMoveSpeed();
	auto distance = myhero->Position().distance(reticles.front().q_reticle->EndPos());
	auto reach_time_normal = time + (((distance - 50) / movespeed) * 1000.0f);
	auto reach_time_w = time + (((distance - 50) / (myhero->GetRawMovementSpeed(movespeed * 0.5) * 0.5f + 230.0f)) * 1000.0f);

	if (is_moving && distance < 100.0f) {
		orbwalker->SetOrbwalkingPoint(nullptr);
	}
	else if (is_moving && myhero->CurrentMana() >= myhero->W()->GetManaCosts() && mouse_pos.distance(*reticle_pos.get()) < draven_catch_range) {
		if (settings->combo.usew && reach_time_normal > reticles.front().time && reach_time_w < reticles.front().time && myhero->W()->isReady())
			myhero->W()->Cast();

		if(settings->enable_evading)
			myhero->ignoreEvade(true);

		orbwalker->SetOrbwalkingPoint(reticle_pos.get());
	}
	is_moving = false;
}

/*
 * E_Logic
 */
void E_Logic() {

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->E()->CastRange());

	if (target == nullptr)
		return;
	
	if (myhero->inRange(target.get(), myhero->E()->CastRange())) {
		auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->E()->MissileSpeed(), myhero->E()->CastRange() - target->BoundingRadius() - 50.0f, myhero->E()->CastDelay()/1000.0f
		);
		if(pos.isValid())
			myhero->E()->Cast(pos);
	}
}

/*
 * R_Logic
 */
void R_Logic() {

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, min_r_distance);

	if (target == nullptr)
		return;

	auto r_damage = 175 + 100 * (myhero->R()->Level() - 1) + myhero->BonusAttackDamage() * 1.1f;
	
	if (myhero->CurrentMana() >= myhero->R()->GetManaCosts() && r_damage > target->CurrentHealth() && myhero->inRange(target.get(), min_r_distance)) {
		auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->R()->MissileSpeed(), myhero->R()->CastRange() - target->BoundingRadius() - 50.0f, myhero->R()->CastDelay() / 1000.0f
		);
		if (pos.isValid())
			myhero->R()->Cast(pos);
	}
}

/*
 * Combo
 */
void Combo() {
	//std::cout << "Combo" << std::endl;
	//	if (on_before_attack && myhero->didAttack())
	//		on_before_attack = false;
	if (catch_axes_mode <= 1)
		AxeCatchingLogic();

	//auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->E()->CastRange());

	//if (target == nullptr)
	//	return;

	//if (settings->combo.usew && myhero->W()->isReady() && myhero->inRange(target.get(), myhero->AttackRange() + 200.0f)) {
	//	myhero->W()->Cast();
	//}
	 if (settings->combo.user && myhero->R()->isReady()) 
		 R_Logic();
	//}
}

/*
 * LastHit
 */
void LastHit() {
	if (catch_axes_mode == 1)
		AxeCatchingLogic();

			auto target = targetselector->GetLastHitTarget(1000.0f);

	if (target == nullptr || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->lasthit.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		myhero->Q()->Cast();
	}
	else if (settings->lasthit.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {
		myhero->W()->Cast();
	
	}
	else if (settings->lasthit.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		myhero->E()->Cast(target);
	}
}
/*
 * Herass
 */
void Herass() {

	LastHit();

	if (catch_axes_mode == 1)
		AxeCatchingLogic();
}

/*
 * LaneClear
 */
void LaneClear() {

	if (catch_axes_mode == 1)
		AxeCatchingLogic();
	else
		orbwalker->SetOrbwalkingPoint(nullptr);

	auto target = targetselector->GetLaneClearTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->laneclear.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
			myhero->Q()->Cast();
	}
	else if (settings->laneclear.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {
		myhero->W()->Cast();
	}
	else if (settings->laneclear.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		myhero->E()->Cast(target);
	}
}



/*
 * Update
 */
void Update() {
	//	bool hasBuff = myhero->hasBuff(buff, "DravenSpinning", true);

	if (orbwalker->Mode() == OrbwalkModes::COMBO) {
		Combo();
	}
	else if (orbwalker->Mode() == OrbwalkModes::HERASS 
		&& myhero->CurrentManaPercent() >= settings->herass.min_mana_percent) {
		Herass();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LANECLEAR 
		&& myhero->CurrentManaPercent() >= settings->laneclear.min_mana_percent) {
		LaneClear();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LASTHIT 
		&& myhero->CurrentManaPercent() >= settings->lasthit.min_mana_percent) {
		LastHit();
	}
}

/*
 * Draw
 */
void Draw() {
	

		if (!myhero->isAlive())
			return;

		if (pos_b.isValid()) {
			drawmanager->DrawCircle3D(pos_b, 15, true, 5, RGBA_COLOR(0, 255, 255, 255), 2);
		}
		/*if (miss != nullptr)
		drawmanager->DrawRect3D(miss->Position(),miss->EndPos(), miss->LineWidth());

		auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 2000.0f);

		if (target == nullptr)
			return;


		auto points = calculatePointsAroundCircle(myhero->Position(), 500.0f, 36);
		auto dist = myhero->Position().distance(target->Position());
		for (auto& p : points) {

			auto dist2 = p.distance(target->Position());

			if(dist2 > dist)
			drawmanager->DrawCircle3D(p, 5, true, 5, RGBA_COLOR(0, 255, 0, 255), 2);
		}

		auto start = target->Position();
		auto end = target->aiPathEnd();
	
		float t = 0;
		auto p = Vector3MovementCollision(start, end, target->MoveSpeed(), myhero->Position(), 1600.0f, t, 250.0f);
		Vector3 pp;
		auto mypoath = target->aiFullPath();
		if (mypoath.size() > 1) {
			//mypoath[0] = start;
			pp = PositionAfter(mypoath, target, t, myhero->MoveSpeed(), 0.0f);
		}
		
		//if(p.x != 0.0f && p.z != 0.0f)
	//	drawmanager->DrawCircle3D(p, 15, true, 5, RGBA_COLOR(0, 255, 255, 255), 2);

		if (pp.x != 0.0f && pp.z != 0.0f)
			drawmanager->DrawCircle3D(pp, 15, true, 5, RGBA_COLOR(150, 150, 150, 255), 2);

		auto poly1 = createRectanglePolyline(myhero->Position(), game->GetMousePosition(), 70.0f);
		auto poly2 = createCirclePolyline(target->Position(), target->BoundingRadius());

		Vector3 point1;
		auto is = checkPolyLineIntersection(point1,poly1, poly2);

		for (auto& a : poly1) {

			
		
			if(PointInPolygon(a, poly2) > 0)
				drawmanager->DrawCircle3D(a, 5, true, 5, RGBA_COLOR(0, 255, 0, 255), 2);
			else
				drawmanager->DrawCircle3D(a, 5, true, 5, RGBA_COLOR(255, 0, 0, 255), 2);
			
			
		}

		for (auto& a : poly2) {
			
			if (PointInPolygon(a, poly1) > 0)
				drawmanager->DrawCircle3D(a, 5, true, 5, RGBA_COLOR(0, 255, 0, 255), 2);
			else
				drawmanager->DrawCircle3D(a, 5, true, 5, RGBA_COLOR(255, 0, 0, 255), 2);

				
		}*/

		//drawmanager->DrawCircle3D(myhero->Position(), 300, false, 60, RGBA_COLOR(0,255, 0, 255), 2);

	/*	for (auto& missile : all_missiles) {
			//	event_handler_manager->Draw(sdk::Events::onMissileCreate, (void*)missile.get());
				//if ((missile->Slot() >= 0 && missile->Slot() < 4 || missile->AnimationName().find("Spell") != std::string::npos) && (missile->MissleTravelBeginTime() + 5) * 1000.0f > reader->GameTime())
			drawmanager->DrawRect3D(missile->StartPos(), missile->EndPos(), missile->LineWidth());
		}*/
		if (draw_axes) {
			if (q_reticles.size() > 0) {
				for (auto& reticle : q_reticles) {
					drawmanager->DrawCircle3D(reticle.second->EndPos(), reticle.second->LineWidth(), false, 20, RGBA_COLOR(0, 255, 0, 255), 2);
				}
			}
		}

		//if(!on_before_attack)
		if (draw_catch_range) {
			Vector3 mpos = game->GetMousePosition();
			drawmanager->DrawCircle3D(mpos, draven_catch_range, false, 60, RGBA_COLOR(0, 255, 255, 255), 2);
		}

		if (draw_e_range) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->E()->CastRange(), false, 60, RGBA_COLOR(255, 0, 100, 255), 2);
		}

		if (draw_r_range) {
			drawmanager->DrawCircle3D(myhero->Position(), 2000, false, 60, RGBA_COLOR(150, 100, 100, 255), 2);
		}
	
}

/*
 * Menu
 */
void Menu() {
	static bool ch = false;

	auto root_node = menu->CreateSubmenu("Draven");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto visuals_node = menu->CreateSubmenu("Axes");
	visuals_node->AddDropDown("Catch Axes", catch_axes_mode, { "only combo", "any mode","never" }, {}, true);
	visuals_node->AddCheckbox("Use W to Catch", &settings->combo.usew);
	visuals_node->AddSliderFloat("Catch Range", draven_catch_range, 100.0, 2000.0, {},true);
	visuals_node->AddCheckbox("Draw Catch Range", &draw_catch_range, {}, true);
	visuals_node->AddCheckbox("Draw Axe Position", &draw_axes, {}, true);
	visuals_node->AddCheckbox("Ignore Evade to catch", &ignore_evade, 
		"Will disable evade to catch axes, otherwise it wont catch axes when the champ is evading", true);

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddSliderInt("Max axes Up", max_axes_count, 0, 2,{}, true);

	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use E On GapCloser", &e_gap_closer);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);
	sub_combo->AddSliderFloat("Min. R Distance", min_r_distance, 1200.0f, 50000.0f, "Minimun distance for Cast R to Target", true);

	sub_combo->addItem(visuals_node);
	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);
	sub_herass->AddCheckbox("Use R", &settings->herass.user);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);
	sub_laneclear->AddCheckbox("Use W", &settings->laneclear.usew);
	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);
	sub_laneclear->AddCheckbox("Use R", &settings->laneclear.user);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use Q", &settings->lasthit.useq);
	sub_lasthit->AddCheckbox("Use W", &settings->lasthit.usew);
	sub_lasthit->AddCheckbox("Use E", &settings->lasthit.usee);
	sub_lasthit->AddCheckbox("Use R", &settings->lasthit.user);

	root_node->addItems({ sub_combo,sub_herass,sub_laneclear,sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
 *	ActiveSpell
 */
void ActiveSpell(std::shared_ptr<GameObjectInstance> sender, std::shared_ptr<ActiveSpellCastInstance> active_spell) {

	//std::cout << "Active: " << active_spell->GetSpellName() << ":" << active_spell->GetSlot() << std::endl;

	/*if (active_spell->GetSpellName() == "MorganaQ") {
		morg_st = active_spell->GetStartPosition();
		morg_ed = active_spell->GetEndPosition();
		auto missile_path = createRectanglePolyline(morg_st, morg_ed, 120.0);
		auto hitbox_path = createCirclePolyline(myhero->Position(), myhero->BoundingRadius());

		for (auto& point : hitbox_path) {
			if (PointInPolygon(point, missile_path) > 0) {
				auto points = calculatePointsAroundCircle(myhero->Position(), 500.0f, 36);
				auto time_to_hit = (morg_st.distance(point) / active_spell->GetMissleSpeed());
				for (auto& safe_point : points) {
					auto a = PositionAfter({ myhero->Position(), safe_point }, myhero, time_to_hit, myhero->MoveSpeed(), 0.0f);
					if (!PointInPolygon(a, missile_path)) {
						std::cout << "Move to..." << std::endl;
						orbwalker->SetOrbwalkingPoint(&a);
						orbwalker->Move();
						return;
					}
				}
			}
		}
	}*/
}

/*
 *	onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->Q()->RegisterSpell(SkillAimType::activate);
	myhero->W()->RegisterSpell(SkillAimType::activate);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	min_r_distance = settings->LoadCustomFloat("Min. R Distance")>0.0f ? settings->LoadCustomFloat("Min. R Distance") : min_r_distance;
	draven_catch_range = settings->LoadCustomFloat("Catch Range")>0.0f ? settings->LoadCustomFloat("Catch Range") : draven_catch_range;
	draw_catch_range = settings->LoadCustomBool("Draw Catch Range");
	draw_axes = settings->LoadCustomBool("Draw Axes");
	max_axes_count = settings->LoadCustomInt("Max axes Up")>0 ? settings->LoadCustomInt("Max axes Up") : max_axes_count;
	catch_axes_mode = settings->LoadCustomInt("Catch Axes")>0 ? settings->LoadCustomInt("Catch Axes") : catch_axes_mode;
	ignore_evade = settings->LoadCustomBool("Ignore Evade to catch");
	e_gap_closer = settings->LoadCustomBool("Use E On GapCloser");
	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");

	Menu();

	event_handler<Events::onCreateObject>::add_callback(CreateObject);
	event_handler<Events::onDeleteObject>::add_callback(DeleteObject);
	event_handler<Events::onUpdate>::add_callback(Update);
	event_handler<Events::onBeforeAttack>::add_callback(BeforeAttack);
	event_handler<Events::onAfterAttack>::add_callback(AfterAttack);
	event_handler<Events::onSpellCast>::add_callback(SpellCast);
	event_handler<Events::onMissileCreate>::add_callback(MissileCreate);
	event_handler<Events::onMissileRemoved>::add_callback(MissileRemove);
	event_handler<Events::onGainBuff>::add_callback(BuffGain);
	event_handler<Events::onLoseBuff>::add_callback(BuffLose);
	event_handler<Events::onGapCloser>::add_callback(GapCloser);

	event_handler<Events::onActiveSpellCast>::add_callback(ActiveSpell);

	event_handler<Events::onDraw>::add_callback(Draw);
}

void onUnload() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " unLoaded" << std::endl;

	event_handler<Events::onCreateObject>::remove_handler(CreateObject);
	event_handler<Events::onDeleteObject>::remove_handler(DeleteObject);
	event_handler<Events::onUpdate>::remove_handler(Update);
	event_handler<Events::onBeforeAttack>::remove_handler(BeforeAttack);
	event_handler<Events::onAfterAttack>::remove_handler(AfterAttack);
	event_handler<Events::onSpellCast>::remove_handler(SpellCast);
	event_handler<Events::onMissileCreate>::remove_handler(MissileCreate);
	event_handler<Events::onMissileRemoved>::remove_handler(MissileRemove);
	event_handler<Events::onGainBuff>::remove_handler(BuffGain);
	event_handler<Events::onLoseBuff>::remove_handler(BuffLose);
	event_handler<Events::onGapCloser>::remove_handler(GapCloser);
	event_handler<Events::onActiveSpellCast>::remove_handler(ActiveSpell);
	event_handler<Events::onDraw>::remove_handler(Draw);
}