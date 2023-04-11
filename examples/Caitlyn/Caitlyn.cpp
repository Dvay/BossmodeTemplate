#include "pch.h"
#include <map>
#include <set>
#include <queue>
#include <iomanip>

using namespace sdk;

static std::string championName = "Caitlyn";
static std::string championVersion = "0.0.6";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreCaitlyn");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;

// Misc checks
static bool cast_w_stuned_enemy = false;
static float r_damage[3] = { 300.0f, 525.0f, 750.0f };
static int save_trap_for_e = 1;
static bool e_gap_closer = false;

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
void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) {

}

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
* Q_Logic
*/
void Q_Logic(std::shared_ptr<GameObjectInstance>& target) {
	if (myhero->CurrentMana() >= myhero->Q()->GetManaCosts()) {
		auto prediction = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(),
			myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0);

		if (prediction.isValid())
			myhero->Q()->Cast(prediction);
	}
}

/*
* W_Logic
*/
void W_Logic(std::shared_ptr<GameObjectInstance>& target) {

	if (myhero->CurrentMana() >= myhero->W()->GetManaCosts()
		&& myhero->W()->isReady() && myhero->inRange(target.get(), myhero->W()->CastRange())) {

		auto predictionW = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->W()->MissileSpeed(),
			myhero->W()->CastRange(), myhero->W()->CastDelay() / 1000.0);

		if (cast_w_stuned_enemy && target->immobilized({ BuffTypes::Snare , BuffTypes::Stun , BuffTypes::Knockup , BuffTypes::Taunt , BuffTypes::Suppression })) {
			if (predictionW.isValid())
				myhero->W()->Cast(predictionW);
		}
		else {
			if (predictionW.isValid())
				myhero->W()->Cast(predictionW);
		}
	}
}

/*
* E_Logic
*/
bool E_Logic(std::shared_ptr<GameObjectInstance> & target) {

	/*
	auto position = target->aiServerPosition();
	auto myhero_pos = myhero->Position();
	auto norm_direction_hero_target = target->Position().direction(myhero_pos).normalize();
	auto E_Pos1 = position + norm_direction_hero_target.scale(200.0f + target->BoundingRadius() / 2.0f);
	*/

	auto prediction = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->E()->MissileSpeed(), myhero->E()->CastRange(), myhero->E()->CastDelay() / 1000.0);
	if (prediction.isValid()) {
	
		auto collide = predictionmanager->WillCollide(myhero->Position(), prediction, myhero->E()->LineWidth(),target.get(),true,true);
		if (!collide) {
			myhero->E()->Cast(target);
			return true;
		}
	}
	return false;
	
}

/*
* Combo
*/
void Combo() {

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 3000.0f);

	if (target == nullptr)
		return;
	/*
	static auto doQ = false;
	static auto doW = false;
	static auto doGaleforce = false;
	*/
	static auto didAA = false;
	auto has_w_buff = target->hasBuff(buff, "caitlynwsnare");
	auto has_e_buff = target->hasBuff(buff, "eternals_caitlyneheadshottracker");

	auto r_damageRaw = 0;
	if(myhero->R()->Level()>0)
		r_damageRaw = r_damage[myhero->R()->Level() - 1] + myhero->BonusAttackDamage()*2;
	
	if (settings->combo.usee && myhero->CurrentMana() >= myhero->E()->GetManaCosts() && myhero->E()->isReady() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange()-80.0f)) {
	
		if (E_Logic(target)) {
			myhero->E()->Cast(target);
		}
	}	
	// for cast E+W
	else if (settings->combo.usew && myhero->aiIsDashing()) {
		W_Logic(target);
	}
	// cast W only when you have more than 1 charge so it will save a W for E+W combo
	else if (settings->combo.usew && myhero->W()->CaitCharges()> save_trap_for_e 
		&& myhero->CurrentMana() >= myhero->W()->GetManaCosts()
		&& (game->GameTime() - myhero->W()->GetLastTimeCasted()>2500.0f) 
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {
		W_Logic(target);
	}
	// Cast Q 
	else if (settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange()-20.0f)) {
		Q_Logic(target);
	}


	if(!didAA && myhero->CanAttack() && myhero->inRange(target.get(), 1300.0f)){
		if (has_w_buff || has_e_buff) {
			myhero->Attack(target);
	
			didAA = true;
		}
	}

	if (didAA && !has_e_buff)
		didAA = false;

	if ((!has_e_buff && !has_w_buff) && !myhero->inRange(target.get(), myhero->Q()->CastRange()-250.0f) 
		&& settings->combo.user && myhero->CurrentMana() >= myhero->R()->GetManaCosts() && myhero->R()->isReady()
		&& myhero->EnemysInRange(500.0f, target->Position()) - 1 < 2
		&& myhero->inRange(target.get(), myhero->R()->CastRange()) 
		&& myhero->TotalOverallDamage(target.get(), r_damageRaw, 0.0f) > target->CurrentHealth()) {
				
		myhero->R()->Cast(target);
	}


	if (myhero->triggered_semi_cast_q && settings->combo.use_semi_q && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange() - 20.0f)) {
		Q_Logic(target);
	}


	if (myhero->triggered_semi_cast_e && settings->combo.use_semi_e && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->E()->CastRange() - 20.0f)) {
		
		E_Logic(target);
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
		&& myhero->inRange(target.get(), myhero->Q()->CastRange() - 20.0f)) {
		myhero->Q()->Cast(target);
	}
}

/*
* Herass
*/
void Herass() {
	LastHit();
	
	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 1000.0f);
	
	if (target == nullptr)
		return;

	if (settings->herass.usew && myhero->W()->isReady() && myhero->W()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->W()->CastRange()))
	{
		W_Logic(target);
	}
	else if (settings->herass.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange() - 20.0f)) {
		Q_Logic(target);
	}
	
}


/*
* LaneClear
*/
void LaneClear() { 

	auto target = targetselector->GetLaneClearTarget(myhero->Q()->CastRange());

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->laneclear.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange() - 20.0f)) {
		myhero->Q()->Cast(target);
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
	
}

/*
* Menu
*/
void Menu() {
	auto root_node = menu->CreateSubmenu("CoreCaitlyn");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Use Semi Q", &settings->combo.use_semi_q);
	sub_combo->AddCheckbox("Use Semi E", &settings->combo.use_semi_e);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use W On Immobilized", &cast_w_stuned_enemy, {}, true);

	auto misc_combo = menu->CreateSubmenu("Misc");
	misc_combo->AddSliderInt("Save Traps for combo", save_trap_for_e, 1,5, "Save Ammount of W for E+W combo", true);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use E On GapCloser", &e_gap_closer);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);

	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");

	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node, misc_combo });

	menu->addItem(root_node);
}

/*
* onLoad
*/
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;


	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target, 0.625f);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target, 0.15f, 750.0f, 0.0f, 1600.0f, 80.0f );
	myhero->R()->RegisterSpell(SkillAimType::mouse_on_target, 0.25f);
	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	save_trap_for_e = settings->LoadCustomInt("Save Traps for combo") > 0 ? settings->LoadCustomInt("Save Traps for combo") : save_trap_for_e;
	 
	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");
	e_gap_closer = settings->LoadCustomBool("Use E On GapCloser");

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