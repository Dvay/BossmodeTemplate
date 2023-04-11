#include "pch.h"
#include <Windows.h>

using namespace sdk;

static std::string championName = "Irelia";
static std::string championVersion = "0.0.9";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreIrelia");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

// Draw Toggle
bool draw_q_range = false;
bool draw_w_range = false;
bool draw_e_range = false;
bool draw_r_range = false;

// Misc  checks
float w_charge_time = 750.0f;
bool q_creeps_while_combo = false;
bool killsteal = false;
bool did_cast_E1 = false;
int use_r_on_x_enemies = 2;
bool q_always = false;
bool dont_q_tower = true;

/*
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

Vector3 posb;
void GapCloser(GameObjectInstance* sender) {
	if (orbwalker->Mode() == OrbwalkModes::NONE)
		return;



}

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
 * Q_CanKilleMinion
 */
bool Q_CanKilleMinion(std::shared_ptr<GameObjectInstance> unit) {
	float dmg = (5.0f + (myhero->Q()->Level() - 1) * 20.0f) + (43.0f + 12.0f * myhero->Level()) + (0.6f * (myhero->BaseDamage() + myhero->BonusAttackDamage()));
	dmg = dmg * (100.0f / (100.0f + unit->FullArmor()));

	return dmg > unit->CurrentHealth();
}

/*
 * Q_CanKilleHero
 */
bool Q_CanKilleHero(std::shared_ptr<GameObjectInstance> unit) {
	float dmg = (5.0f + (myhero->Q()->Level() - 1) * 20.0f) + (0.6f * (myhero->BaseDamage() + myhero->BonusAttackDamage()));
	dmg = dmg * (100.0f / (100.0f + unit->FullArmor()));

	return dmg > unit->CurrentHealth();
}

/*
 * GetKillableCreepInAARange
 */
Vector3 GetKillableCreepInAARange(std::shared_ptr<GameObjectInstance> current_target) {
	GameObjectInstance* tower = nullptr;
	for (auto& object : targetselector->m_all_units) {
		auto obj = object.second->Position();

		if (!object.second->isValid() || object.second->isAlly() || current_target->Position().distance(obj) > myhero->AttackRange() + current_target->BoundingRadius() || (object.second->isInEnemyTowerRange(tower) && tower != nullptr && tower->isValid() 
			&& tower->AllysInRange(tower->BaseAttackRange()) == 0))
			continue;

		if (Q_CanKilleMinion(object.second)) {
			return obj;
		}
	}
	return Vector3(0.0f, 0.0f, 0.0f);
}

/*
 * Killsteal
 */
Vector3 Killsteal() {
	for (auto& object : targetselector->m_all_heros) {
		auto obj = object->Position();

		if (!object->isValid() || object->isAlly() || obj.distance(myhero->Position()) > myhero->Q()->CastRange())
			continue;

		if (Q_CanKilleHero(object)) {
			return obj;
		}
	}
	return Vector3(0.0f, 0.0f, 0.0f);
}

/*
 * GetDashableCreep
 */
Vector3 GetDashableCreep(Vector3  from, Vector3 to, float angle) {
	for (auto& object : targetselector->m_all_units) {
		auto obj = object.second->Position();

		if (!object.second->isValid() || object.second->isAlly() || from.distance(obj) > myhero->Q()->CastRange() - object.second->BoundingRadius())
			continue;

		if (predictionmanager->isBetween(from, to, obj, angle) && (Q_CanKilleMinion(object.second) || (myhero->CurrentHealth() / myhero->MaxHealth()) < 0.15)) {
			return obj;
		}
	}
	return Vector3();
}

/*
 * GetEnemiesInRange
 */
int GetEnemiesInRange(Vector3& pos, float range) {
	int count = 0;
	for (auto& enemy : targetselector->m_all_heros) {
		if (!enemy->isValid() || enemy->isAlly())
			continue;

		if (pos.distance(enemy->Position()) < range) {
			count++;
		}
	}
	return count;
}

/*
 * TargetWithIreliaMark
 */
Vector3 TargetWithIreliaMark() {
	for (auto& enemy : targetselector->m_all_heros) {
		if (!enemy->isValid() || enemy->isAlly())
			continue;

		if (myhero->inRange(enemy.get(), myhero->Q()->CastRange() - enemy->BoundingRadius()) && enemy->hasBuff(buff, "ireliamark")) {
			return enemy->Position();
		}
	}
	return Vector3();
}

/*
 * Combo
 */
void Combo() {
	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 1000.0f);

	if (target == nullptr) {
		if (myhero->W()->isCharging()) {
			myhero->W()->Cast(game->GetMousePosition(), w_charge_time);
		}
		return;
	}

	GameObjectInstance * turret;


	if (settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()) 
	{
		if (settings->combo.user && !Q_CanKilleHero(target)) {
			auto Q_Target = TargetWithIreliaMark();

			

			if (Q_Target.isValid()) {

				auto in_turret_range = myhero->isInEnemyTowerRange(turret, Q_Target);

				if(!dont_q_tower || !in_turret_range)
				myhero->Q()->Cast(Q_Target);
			}
		}

		auto dashable_creep = GetDashableCreep(myhero->Position(), game->GetMousePosition(), 15.0f);
		if (dashable_creep.isValid() && (dashable_creep.distance(target->Position()) < myhero->Position().distance(target->Position()))) {
						
				auto in_turret_range = myhero->isInEnemyTowerRange(turret, dashable_creep);
				if (!dont_q_tower || !in_turret_range)
				myhero->Q()->Cast(dashable_creep);
			
		}
	}

	if (killsteal && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()) {
		auto killable_hero = Killsteal();

		if (killable_hero.isValid()) {
			auto in_turret_range = myhero->isInEnemyTowerRange(turret, killable_hero);
			if (!dont_q_tower || !in_turret_range)
			myhero->Q()->Cast(killable_hero);
		}
	}

	if (q_creeps_while_combo && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && !Q_CanKilleHero(target)) {
		auto killable_creep = GetKillableCreepInAARange(target);
		if (killable_creep.isValid() && settings->combo.useq) {
			auto in_turret_range = myhero->isInEnemyTowerRange(turret, killable_creep);

		
			if (!dont_q_tower || !in_turret_range)
			myhero->Q()->Cast(killable_creep);
		}
	}

	if (q_always && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && myhero->inRange(target.get(), myhero->Q()->CastRange())) {
			
		auto in_turret_range = myhero->isInEnemyTowerRange(turret, target->Position());

	//	std::cout << in_turret_range << std::endl;
		if (!dont_q_tower || !in_turret_range)
		myhero->Q()->Cast(target);
	}

		

	if (settings->combo.useq && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && myhero->Q()->isReady() && myhero->inRange(target.get(), myhero->Q()->CastRange() - target->BoundingRadius()) 
		&& (Q_CanKilleHero(target) || ((target->CurrentHealth() < (myhero->TotalPhysicalDamage(target.get()) * 2.0f)) && !myhero->inRange(target.get(), myhero->AttackRange()+150.0f)))) {


		auto in_turret_range = myhero->isInEnemyTowerRange(turret, target->Position());
		if (!dont_q_tower || !in_turret_range)
		myhero->Q()->Cast(target);
	}
	else if (settings->combo.usee && myhero->CurrentMana() >= myhero->E()->GetManaCosts() && myhero->E()->isReady() 
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())	&& myhero->E()->BaseCooldown() != 0.25) {
		myhero->E()->Cast(myhero->Position());
	}
	else if (settings->combo.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->E()->CastRange())	&& myhero->E()->BaseCooldown() == 0.25) {


		auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->E()->MissileSpeed(), myhero->E()->CastRange(), myhero->E()->CastDelay() / 1000.0f);

	//	auto from = target->Position();
		//pos = from.extend(pos, 150.0f);

		if (pos.isValid()) {
			myhero->E()->Cast(pos);
		}
	}
	else if (settings->combo.usew && myhero->CurrentMana() >= myhero->W()->GetManaCosts() && !Q_CanKilleHero(target) 
		&& myhero->W()->isReady() && !myhero->W()->isCharging()
		&& myhero->inRange(target.get(), myhero->W()->CastRange() - target->BoundingRadius() - 400.0f)) {
		myhero->W()->Cast(target, w_charge_time);
	}
	else if (settings->combo.usew && myhero->CurrentMana() >= myhero->W()->GetManaCosts() && !Q_CanKilleHero(target) 
		&& myhero->W()->isReady() && myhero->W()->isCharging()
		&& myhero->inRange(target.get(), myhero->W()->CastRange() - target->BoundingRadius() - 50.0f)) {
		myhero->W()->Cast(target, w_charge_time);
	}
	else if (settings->combo.user && myhero->CurrentMana() >= myhero->R()->GetManaCosts() && myhero->R()->isReady() 
		&& myhero->inRange(target.get(), myhero->R()->CastRange() - target->BoundingRadius() - 200.0f)) {
		auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->R()->MissileSpeed(), myhero->R()->CastRange(), myhero->R()->CastDelay() / 1000.0f);

		int count = GetEnemiesInRange(pos, 350);

		if (count >= use_r_on_x_enemies && pos.isValid())
			myhero->R()->Cast(pos);
	}
}

/*
 * Herass
 */
void Herass() { }

/*
 * LaneClear
 */
void LaneClear() {
	auto target = targetselector->GetLaneClearTarget(myhero->Q()->CastRange());

		if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	float dmg = (5.0f + (myhero->Q()->Level() - 1) * 20.0f) + (43.0f + 12.0f * myhero->Level()) + (0.6f * (myhero->BaseDamage() + myhero->BonusAttackDamage()));
	//dmg = dmg * (100.0f / (100.0f + target->FullArmor()));

	dmg = myhero->TotalOverallDamage(target.get(), dmg, 0.0f);
	/// <summary>
	/// Damage Taken = Damage Dealt * (100 / (100 + Armor))
	/// </summary>
	///
	///

	//std::cout << target->ChampionName() << ": " << dmg << " : " << target->BoundingRadius() << std::endl;
	GameObjectInstance* tower = nullptr;
	if (settings->laneclear.useq && myhero->Q()->isReady() && myhero->inRange(target.get(), myhero->Q()->CastRange()) && dmg > target->CurrentHealth()) {
		myhero->Q()->Cast(target);
	}
}

/*
 * LastHit
 */
void LastHit() { 

		auto target = targetselector->GetLastHitTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->laneclear.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		if (myhero->Q()->AimTtype() != SkillAimType::activate)
			myhero->Q()->Cast(target);
		else
			myhero->Q()->Cast();
	}
	else if (settings->laneclear.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {
		if (myhero->W()->AimTtype() != SkillAimType::activate)
			myhero->W()->Cast(target);
		else
			myhero->W()->Cast();
	}
	else if (settings->laneclear.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		if (myhero->E()->AimTtype() != SkillAimType::activate)
			myhero->E()->Cast(target);
		else
			myhero->E()->Cast();
	}
	else if (settings->laneclear.user && myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->R()->CastRange())) {
		if (myhero->R()->AimTtype() != SkillAimType::activate)
			myhero->R()->Cast(target);
		else
			myhero->R()->Cast();
	}
}

/*
 * Flee
 */
void Flee() {
	auto creep = GetDashableCreep(myhero->Position(), game->GetMousePosition(), 15.0f);

	if (creep.isValid()) {
		if (settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()) {
			myhero->Q()->Cast(creep);
		}
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
	else if (orbwalker->Mode() == OrbwalkModes::FLEE) {
		Flee();
	}
}

/*
 * Draw
 */
void Draw() {
	
		auto pos = myhero->Position();

		if (!myhero->isAlive())
			return;


	
		

		if (draw_q_range && myhero->Q()->Level() > 0) {
			drawmanager->DrawCircle3D(pos, myhero->Q()->CastRange(), false, 60, RGBA_COLOR(135, 0, 100, 255), 2);
		}

		if (draw_w_range && myhero->W()->Level() > 0) {
			drawmanager->DrawCircle3D(pos, myhero->W()->CastRange(), false, 60, RGBA_COLOR(140, 142, 100, 255), 2);
		}

		if (draw_e_range && myhero->E()->Level() > 0) {
			drawmanager->DrawCircle3D(pos, myhero->E()->CastRange(), false, 60, RGBA_COLOR(78, 142, 100, 255), 2);
		}

		if (draw_r_range && myhero->R()->Level() > 0) {
			drawmanager->DrawCircle3D(pos, myhero->R()->CastRange(), false, 60, RGBA_COLOR(150, 100, 100, 255), 2);
		}

		//drawmanager->DrawCircle3D(pos, 30, true, 20, RGBA_COLOR(0, 222, 120, 255), 2);
		//	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 3000.0f);

		//if (target == nullptr)
		//	return;

		//std::cout << p.castPosition.x << " : " << p.castPosition.y << " : " << p.castPosition.z << " : " << p.hitChance << " : " << p.maxHits << std::endl;

		//	auto from = myhero->Position();

		//	auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->R()->MissileSpeed(), myhero->R()->CastRange(), myhero->R()->CastDelay());

		//	castPosCenter = t_pos.add(target->Position().direction(t_pos).invert().normalize().scale(target->BoundingRadius() - 10));
		//direction = target->Position().direction(t_pos).perpendicular(true).normalize();

		//direction = target->Position().direction(t_pos).perpendicular().normalize();

		//	auto extended1 = castPosCenter;// .add(direction.scale(100.0));

		//if (pos.isValid()) {
			//	drawmanager->DrawCircle3D(castPosCenter, 20, true, 20, RGBA_COLOR(255, 0, 120, 255), 2);
				//drawmanager->DrawCircle3D(castPosCenter2, 20, true, 20, RGBA_COLOR(0, 255, 120, 255), 2);
		//}
	
}

/*
* Menu
*/
void Menu() {

	auto root_node = menu->CreateSubmenu("CoreIrelia");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto sub_combo = menu->CreateSubmenu("Combo");

	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	auto q_node = menu->CreateSubmenu("Q Usage");
	q_node->AddCheckbox("Q Always", &q_always, {}, true);
	q_node->AddCheckbox("Q KillSteal", &killsteal, {}, true);
	q_node->AddCheckbox("Q Creeps while Combo", &q_creeps_while_combo, {}, true);
	q_node->AddCheckbox("Dont Q under tower", &dont_q_tower, {}, true);
	sub_combo->addItem(q_node);

	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);
	sub_combo->AddSliderInt("Use on X Enemies >=", use_r_on_x_enemies, 1, 5);

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

	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
 * onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target, 0.15f);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target, 0.25f, 0.0, 0.0, 2000.0f, 90.0f);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	killsteal = settings->LoadCustomBool("Q KillSteal");
	use_r_on_x_enemies = settings->LoadCustomInt("Q Creeps while Combo") > 0 ? settings->LoadCustomInt("Q Creeps while Combo") : use_r_on_x_enemies;

	q_always  = settings->LoadCustomBool("Q Always");
	killsteal = settings->LoadCustomBool("Q KillSteal");
	q_creeps_while_combo = settings->LoadCustomBool("Q Creeps while Combo");
	dont_q_tower = settings->LoadCustomBool("Dont Q under tower");



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