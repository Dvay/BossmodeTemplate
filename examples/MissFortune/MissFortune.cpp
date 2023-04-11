#include "pch.h"
using namespace sdk;

static std::string championName = "MissFortune";
static std::string championVersion = "0.1.1";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreMissFortune");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

// Q Damage
static float q_dmg[5] = { 20.0f, 45.0f, 70.0f, 95.0f, 120.0f };
static float q_angle = 35.0f;

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;
static bool draw_q_cone = false;

// Misc  checks
float min_r_health = 450.0f;
bool use_r_gapcloser = false;
//bool lasthit_with_herass = false;
bool use_killable_kreep_only = false;

/*
* 
 * BeforeAttack
 */
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;
}

/*
 * AfterAttack
 */
void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) { }

/*
 * SpellCast
 */
void SpellCast(GameObjectInstance* sender) { }

/*
 * GapCloser
 */
void GapCloser(GameObjectInstance* sender) { }

/*
 * MissileCreate
 */
void MissileCreate(std::shared_ptr<MissileInstance> missile) { }

/*
 * MissileRemove
 */
void MissileRemove(std::shared_ptr<MissileInstance> missile) { }

/*
 * BuffGain
 */
void BuffGain(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() == myhero->Index())
		return;

	//std::cout << "Buff Name: " << buff->Name() << std::endl;
}

/*
 * BuffLose
 */
void BuffLose(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() != myhero->Index())
		return;
}

/*
 * CreateObject
 */
void CreateObject(std::shared_ptr<GameObjectInstance> obj) { }

/*
 * DeleteObject
 */
void DeleteObject(std::shared_ptr<GameObjectInstance> obj) { }

/*
 * WillCollide
 */
bool WillCollide(const Vector3& from, const Vector3& to, int networkid, float missile_width) {
	for (auto& obj : targetselector->m_all_units) {
		if (!obj.second->isValid() || obj.second->isAlly() || networkid == obj.second->NetworkId())
			continue;

		Vector3 colliding_point(0, 0, 0);
		
		if (predictionmanager->isObjectOnMissilePath(colliding_point, from, to, obj.second.get(), missile_width)) {
			return true;
		}
	}
	return false;
}

void Q_Logic()
{

	if (!myhero->Q()->isReady())
		return;

	auto myhero_pos = myhero->Position();


	auto q_damage = q_dmg[myhero->Q()->Level() - 1] + myhero->TotalRawAttackDamage() + (myhero->AbilityPower() * 0.35);
	
	std::shared_ptr < GameObjectInstance> saved_target;
	std::shared_ptr < GameObjectInstance> saved_creep;
	float temp_health = FLT_MAX;
	bool did_find_creep = false;

	for (auto& enemy_hero : targetselector->m_all_heros) {

		if (!enemy_hero->isValid() || enemy_hero->isAlly() )
			continue;

		auto enemy_pos = enemy_hero->Position();
		
		if (!use_killable_kreep_only) {
			auto enemy_health = enemy_hero->CurrentHealth();
			if (myhero->inRange(enemy_hero.get(), myhero->Q()->CastRange()) && enemy_health < temp_health) {

				saved_target = enemy_hero;
				temp_health = enemy_health;
			}
		}
	

		for (auto& unit : targetselector->m_all_units) {

			if (!unit.second->isValid() || unit.second->isAlly() || !myhero->inRange(unit.second.get(), myhero->AttackRange()) || unit.second->NetworkId() == enemy_hero->NetworkId() || !unit.second->inRange(enemy_hero.get(), 450.0f))
				continue;

	
			auto total_damage = myhero->TotalOverallDamage(unit.second.get(), q_damage, 0.0);
			
			//auto unit_pos = unit.second->Position();
			auto unit_pos = unit.second->Position();
			auto vector_myhero_to_unit = unit_pos.direction(myhero_pos).normalize();

			auto rotated1 = vector_myhero_to_unit.Rotated((q_angle * (3.1415926f / 180.0f))).normalize();
			auto rotated2 = vector_myhero_to_unit.Rotated(((360.0 - q_angle) * (3.1415926f / 180.0f))).normalize();


			auto vector_unit_to_enemy = enemy_pos.direction(unit_pos).normalize();

			auto angle1 = rotated1.angel_between(vector_unit_to_enemy);
			auto angle2 = rotated2.angel_between(vector_unit_to_enemy);
		//	std::cout << angle << std::endl;
		
			if (angle1 < (q_angle*2.0f) && angle2 < (q_angle * 2.0f)&& !WillCollide(unit_pos, enemy_pos, unit.second->NetworkId(), myhero->Q()->LineWidth())) {
				
				if (unit.second->CurrentHealth() < total_damage) {
					myhero->Q()->Cast(unit.second);
					return;
				}
				else
				{
		
					did_find_creep = true;
					saved_creep = unit.second;
				}
			}


		}

	}

	if (!use_killable_kreep_only) {
		if (saved_target != nullptr && saved_target->isValid()) {
			
			myhero->Q()->Cast(saved_target);
			return;
		}


		if (did_find_creep && saved_creep != nullptr && saved_creep->isValid()) {
		
			myhero->Q()->Cast(saved_creep);
			return;
		}
	}

}

/*
* E_logic
*/
void E_Logic(std::shared_ptr<sdk::GameObjectInstance> target)
{
	if (target == nullptr)
		return;

	if (myhero->CurrentMana() >= myhero->E()->GetManaCosts()) {
		myhero->E()->Cast(target);
	}
}

/*
 * Combo
 */
void Combo() {

	static bool did_ulti = false;

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 3500.0f);

	if (target == nullptr)
		return;

	bool has_r_up = false;
	if (myhero->R()->Level() >= 1) {
		 has_r_up = myhero->hasBuff(buff, "missfortunebulletsound");

		 if (did_ulti) {
			 myhero->R()->Cast(target);
			return;
		 }

		if (did_ulti || has_r_up) {
			did_ulti = false;
			return;
		}

		if (!did_ulti && !has_r_up && !myhero->isCasting())
			orbwalker->DisableMovement(false);
	}
	
	if(settings->combo.useq)
		Q_Logic();

	// W
	if (settings->combo.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts()
		&& !myhero->inRange(target.get(), myhero->AttackRange()) && myhero->inRange(target.get(), myhero->R()->CastRange())) {
		myhero->W()->Cast();
	}
	// E
	else if (settings->combo.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		E_Logic(target);
	}
	// R
	else if (!has_r_up && settings->combo.user && myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->R()->CastRange()-200.0f)
		&& target->CurrentHealth() <= min_r_health
		) {

		myhero->R()->Cast(target);

		orbwalker->DisableMovement(true);
		did_ulti = true;
		
	}
}

/*
 * LastHit
 */
void LastHit() {


	if (settings->lasthit.useq)
		Q_Logic();

	auto target = targetselector->GetTarget(TargetSelectorTypes::distance_to_champ, 1000.0f);

	if (target == nullptr)
		return;

	if (settings->lasthit.usee && myhero->inRange(target.get(), myhero->E()->CastRange()))
		E_Logic(target);

}
/*
 * Herass
 */
void Herass() {

	LastHit();

	auto target = targetselector->GetTarget(TargetSelectorTypes::distance_to_champ, 1000.0f);

	if (target == nullptr)
		return;
	
	if (settings->herass.useq)
		Q_Logic();
	else if (settings->herass.usee)
		E_Logic(target);
}



/*
 * LaneClear
 */
void LaneClear() {
	
	if (settings->laneclear.useq) {
		Q_Logic();
	}

	auto target = targetselector->GetLaneClearTarget(1000.0f);
	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->laneclear.usee && myhero->inRange(target.get(), myhero->E()->CastRange())) {
		E_Logic(target);
	}
}

/*
 * Update
 */
void Update() {
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

		if (draw_q_range && myhero->Q()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->Q()->CastRange(), false, 60, RGBA_COLOR(135, 0, 100, 255), 2);
		}

		if (draw_w_range && myhero->W()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->W()->CastRange(), false, 60, RGBA_COLOR(140, 142, 100, 255), 2);
		}

		if (draw_e_range && myhero->E()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->E()->CastRange(), false, 60, RGBA_COLOR(78, 142, 100, 255), 2);
		}

		if (draw_r_range && myhero->R()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->R()->CastRange(), false, 60, RGBA_COLOR(150, 100, 100, 255), 2);
		}

		if (draw_q_cone) {

			auto myhero_pos = myhero->Position();
			int num_segments = 20;
			for (auto& enemy_hero : targetselector->m_all_heros) {

				if (!enemy_hero->isValid() || enemy_hero->isAlly() || !myhero->inRange(enemy_hero.get(), myhero->Q()->CastRange()))
					continue;


				auto hero_pos = enemy_hero->Position();
				auto direction = hero_pos.direction(myhero_pos);

				auto rotated1 = direction.Rotated((q_angle * (3.1415926f / 180.0f))).normalize();
				auto rotated2 = direction.Rotated(((360.0 - q_angle) * (3.1415926f / 180.0f))).normalize();


				auto end1 = hero_pos.add(rotated1 * 410.0f);
				auto end2 = hero_pos.add(rotated2 * 410.0f);


				drawmanager->DrawLine(hero_pos, end1, 1.0f, RGBA_COLOR(0, 255, 100, 255));
				drawmanager->DrawLine(hero_pos, end2, 1.0f, RGBA_COLOR(0, 255, 100, 255));

				float angle_between = rotated1.angel_between(rotated2);
				float angle_increment = angle_between / (num_segments - 1);
				std::vector<Vector3> segment_points;

				for (int i = 0; i < num_segments; i++) {
					float theta = angle_increment * i;
					auto rotated = rotated1.Rotated(((360.0 - theta) * (3.1415926 / 180.0))).normalize();
					auto end3 = hero_pos.add(rotated * 410.0f);
					segment_points.push_back(end3);
				}

				drawmanager->DrawPolyLine(segment_points, 1.0f, RGBA_COLOR(0, 255, 100, 255));
			}
		}
	
		
	

}
		/*
		// check if enemy

	}
}

/*
 * Menu
 */
void Menu() {

	auto root_node = menu->CreateSubmenu("CoreMissFortune");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);
	drawing_node->AddCheckbox("Draw Q Cone", &draw_q_cone, {}, true);



	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Q on killable only", &use_killable_kreep_only, "This will only use Q when a killable creep can be used to bounce on the Target", true);
	sub_combo->AddSliderFloat("Q Angle", q_angle, 25.0f, 45.0, {}, true);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);
	sub_combo->AddSliderFloat("Use R Healt <=", min_r_health, 100.0f, 2000.0f, {}, true);


	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);
	sub_laneclear->AddCheckbox("Use W", &settings->laneclear.usew);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use W", &settings->lasthit.usew);
	//sub_lasthit->AddCheckbox("Use Herass on lasthit", &lasthit_with_herass, {}, true);

	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
 * onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->Q()->RegisterSpell(SkillAimType::mouse_on_target, 0.0, 0.0, 0.0, 0.0, 70.0f);
	myhero->W()->RegisterSpell(SkillAimType::activate);
	myhero->E()->RegisterSpell(SkillAimType::mouse_on_target);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target, 1.0, 1450.f);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	auto temp_val = settings->LoadCustomFloat("Use R Healt <=");
	if(temp_val > 0.0f)
		min_r_health = temp_val;
		
	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");
	draw_q_cone = settings->LoadCustomBool("Draw Q Cone");
	//lasthit_with_herass = settings->LoadCustomBool("Use Herass on lasthit");
	use_killable_kreep_only = settings->LoadCustomBool("Q on killable only");

	if(settings->LoadCustomFloat("Q Angle") > 0.0f)
		q_angle = settings->LoadCustomFloat("Q Angle");

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

	event_handler<Events::onDraw>::add_callback(Draw);
}

/*
 * onUnload
 */
void onUnload() {

	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " unLoaded" << std::endl;

	event_handler<Events::onCreateObject>::remove_handler(CreateObject);
	event_handler<Events::onDeleteObject>::add_callback(DeleteObject);

	event_handler<Events::onUpdate>::remove_handler(Update);

	event_handler<Events::onBeforeAttack>::remove_handler(BeforeAttack);
	event_handler<Events::onAfterAttack>::remove_handler(AfterAttack);
	event_handler<Events::onSpellCast>::remove_handler(SpellCast);

	event_handler<Events::onMissileCreate>::remove_handler(MissileCreate);
	event_handler<Events::onMissileRemoved>::remove_handler(MissileRemove);

	event_handler<Events::onGainBuff>::remove_handler(BuffGain);
	event_handler<Events::onLoseBuff>::remove_handler(BuffLose);

	event_handler<Events::onGapCloser>::remove_handler(GapCloser);

	event_handler<Events::onDraw>::remove_handler(Draw);
}