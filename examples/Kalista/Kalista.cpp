#include "pch.h"
#include <map>
#include <set>
#include <queue>
#include <iomanip>

using namespace sdk;

static std::string championName = "Kalista";
static std::string championVersion = "0.1.3";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreKalista");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;


// Misc  checks
static float r_dmg = 150.0f;
static bool did_dash = false;
static Vector3 flash_position;
static bool is_dashing_q2 = false;
static int min_e_killable_combo = 1;
static bool save_ally_with_r = false;
static int min_e_killable_laneclear = 1;
static float RawRendDamage[5] = { 20.0f, 30.0f, 40.0f, 50.0f, 60.0f };
static float RawRendDamagePerSpear[5] = { 10.0f, 16.0f, 22.0f, 28.0f, 34.0f };
static float RawRendDamagePerSpearMultiplier[5] = { 0.232f, 0.2755f, 0.319f, 0.3625f, 0.406f };

/*
* R_logic
*/
void R_logic() {
	if (myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts()){
		int enemy_in_range = 0;
		GameObjectInstance* bound = nullptr;
		int r_range = myhero->R()->CastRange();

		for (auto& hero : targetselector->m_all_heros) {

			if (!hero->isAlly() && hero->isValid() && myhero->inRange(hero.get(), 1100.0f))
				enemy_in_range++;

			if (hero->isAlly() && hero->isValid() && hero->hasBuff(buff, "kalistacoopstrikeally")) {
				bound = hero.get();
			}
		}
		// Save Ally from died
		if (bound!= nullptr && enemy_in_range>=1 && myhero->inRange(bound, r_range) && bound->CurrentHealth() <= 300.0f) {
			myhero->R()->Cast();
		}
	}
}

/*
* BeforeAttack
*/
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;
}

/*
* is_killable_by_e
*/
bool is_killable_by_e(std::shared_ptr<GameObjectInstance> target) {
	auto eLvl = myhero->E()->Level() - 1;
	auto dmg = myhero->TotalPhysicalDamage(target.get());
	float full_damage = (RawRendDamage[eLvl] + (0.7f * dmg) + // initial base damage
		((buff->isAlive() - 1) * // isAlive() returns the current Spear count
			(RawRendDamagePerSpear[eLvl] + (RawRendDamagePerSpearMultiplier[eLvl] * dmg))))-20.0f;//  damage per stack

	if (target->is_dragon() || target->is_baron() || target->is_herald())
		full_damage /= 2.0f;


	if (full_damage > (target->CurrentHealth())) {
		return true;
	}

	return false;
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
void CreateObject(std::shared_ptr<GameObjectInstance> obj) {
	//if (obj->ObjectName() == "DravenSpinningReturn") {
		//	std::cout << "Created: " << obj->ObjectName() << " : " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - time1).count()) << std::endl;
//	}
}

/*
* DeleteObject
*/
void DeleteObject(std::shared_ptr<GameObjectInstance> obj) {
	//if (obj->ObjectName().find("Draven") != std::string::npos)
	//std::cout << "Deleted: " << obj->ObjectName() << std::endl;
}


/*
* Combo
*/
void Combo() {

	// E
	if (settings->combo.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()) {
		int count_killables = 0;
		for (auto& enemy_hero : targetselector->m_all_heros) {

			if (enemy_hero->isAlly() || !enemy_hero->isValid())
				continue;
			//auto enemy_hero = shr_enemy_hero.get();

			if (myhero->inRange(enemy_hero.get(), myhero->E()->CastRange()) && enemy_hero->hasBuff(buff, "kalistaexpungemarker")
				&& is_killable_by_e(enemy_hero)) {
				count_killables++;
			}
		}

		if (count_killables >= min_e_killable_combo) {
			myhero->E()->Cast();
		}
	}

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());

	if (target == nullptr)
		return;

	// SEMI Q
	if (myhero->triggered_semi_cast_q && settings->combo.use_semi_q && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		auto prediction = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), 2400.0f, myhero->Q()->CastRange(), 0.250);
		auto collision = predictionmanager->WillCollide(myhero->Position(), target->Position(), myhero->Q()->LineWidth(), target.get(), true, true);
		if (prediction.isValid() && !collision) {
			myhero->Q()->Cast(prediction);
			myhero->triggered_semi_cast_q = false;
		}
	}

	// Q	
	if (settings->combo.useq  && !myhero->isAttacking() && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && myhero->inRange(target.get(), myhero->Q()->CastRange())) {

		auto prediction = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), 2400.0f, myhero->Q()->CastRange(), 0.250);
		auto collision = predictionmanager->WillCollide(myhero->Position(), target->Position(), myhero->Q()->LineWidth(),target.get(), true,true);
		if (prediction.isValid() && !collision)
			myhero->Q()->Cast(prediction);
	}
	// R
	/*else if (settings->combo.user && myhero->CurrentMana() >= myhero->R()->GetManaCosts() && myhero->R()->isReady() && myhero->inRange(target.get(), myhero->R()->CastRange())) {
		myhero->R()->Cast();

	//  Save Ally from die	
	}*/
	else if (save_ally_with_r) {
		R_logic();
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

	if (settings->laneclear.usee && myhero->E()->isReady()) {
		int count_killables = 0;
		for (auto& enemy_creep : targetselector->m_all_units) {

			if (enemy_creep.second->isAlly() || !enemy_creep.second->isValid())
				continue;

			if (myhero->inRange(enemy_creep.second.get(), myhero->E()->CastRange()) && enemy_creep.second->hasBuff(buff, "kalistaexpungemarker")
				&& is_killable_by_e(enemy_creep.second)) {
				count_killables++;
			}
		}
		// if valid count_killables
		if (count_killables >= min_e_killable_laneclear) {
			myhero->E()->Cast();
		}
	}

	auto target = targetselector->GetLaneClearTarget(myhero->E()->CastRange());

		if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;
	
	if (settings->laneclear.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
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

	if (settings->lasthit.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		myhero->Q()->Cast(target);

	}
	else if (settings->lasthit.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		myhero->E()->Cast();
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


	//	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());

	//if (target == nullptr)
		//	return;

	//auto prediction = predictionmanager->GetPredictedPositionSkillShot(target.get(), myhero.get(), 2400.0f, myhero->Q()->CastRange(), 0.250, myhero->Q()->LineWidth());
	//	auto prediction2 = predictionmanager->GetPredictedPositionSkillShot(target.get(), myhero.get(),2400.0f, myhero->Q()->CastRange(), 250, myhero->Q()->LineWidth());
	
		//if(prediction.isValid())
		//	drawmanager->DrawCircle3D(prediction, 10, true, 20, RGBA_COLOR(0, 255, 0, 255), 2);

			//if (prediction2.isValid())
				//	drawmanager->DrawCircle3D(prediction2, 10, true, 20, RGBA_COLOR(255, 0,0, 255), 2);


		//	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());
		//	if (target == nullptr)
		//	return;

		//	auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->CastRange(), myhero->Q()->MissileSpeed(), 0.25f, myhero->Q()->LineWidth());

		//	drawmanager->DrawCircle3D(pos, 30, true, 20, RGBA_COLOR(0, 222,120, 255), 2);
	
}

/*
* Menu
*/
void Menu() {
	auto root_node = menu->CreateSubmenu("CoreKalista");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Use Semi Q", &settings->combo.use_semi_q);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddSliderInt("Min. killables by E", min_e_killable_combo,1,5);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);
	sub_combo->AddCheckbox("Save Ally Bound with R", &save_ally_with_r);

	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);
	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);
	sub_laneclear->AddSliderInt("killables by E min.", min_e_killable_laneclear, 1, 5);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use Q", &settings->lasthit.useq);
	sub_lasthit->AddCheckbox("Use E", &settings->lasthit.usee);

	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
* onLoad
*/
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;


	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target, 0.25, 1200.0f, 0.0f, 2400.0f);
	myhero->W()->RegisterSpell(SkillAimType::activate);
	myhero->E()->RegisterSpell(SkillAimType::activate);
	myhero->R()->RegisterSpell(SkillAimType::activate);
	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);
	
	save_ally_with_r = settings->LoadCustomBool("Save Ally Bound with R");
	min_e_killable_combo = settings->LoadCustomInt("Min. killables by E")>0 ? settings->LoadCustomInt("Min. killables by E") : min_e_killable_combo;
	min_e_killable_laneclear = settings->LoadCustomInt("killables by E min.")>0 ? settings->LoadCustomInt("killables by E min.") : min_e_killable_laneclear;
	
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