#include "pch.h"

using namespace sdk;

static std::string championName = "Jinx";
static std::string championVersion = "0.1.1";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreJinx");
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
float q_rocket_bonus_range[5] = { 80.0f, 110.0f , 140.0f, 170.0f , 200.0f };
static float r_damage[3] = { 300.0f, 450.0f, 70.0f };
static float r_bonus[3] = { 0.25f, 0.30f, 0.35f };
static bool e_gap_closer = false;
static float q_mana = 60.0f;
static float min_r_distance = 2000.0f;
static float w_damage[5] = { 10.0f,60.0f,110.0f,160.0f,210.0f };

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

	if (e_gap_closer) {
		//std::cout << sender->ChampionName() << std::endl;
		auto start_pos = sender->aiPathStart();
		auto end_pos = sender->aiPathEnd();

		
	
	
		auto my_current_pos = myhero->Position();
		auto start_dist = my_current_pos.distance(start_pos);
		auto end_dist = my_current_pos.distance(end_pos);

		if (myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
			&& my_current_pos.distance(end_pos) < 700.0f && start_dist > end_dist) {
			myhero->E()->Cast(end_pos);
		}
	}
}

/*
* MissileCreate
*/
void MissileCreate(std::shared_ptr<MissileInstance> missile) {
	if (missile->SourceIndex() != 161745315 && missile->Slot() != 0)
		return;

	/*if (missile->Team() != myhero->Team() && myhero->Position().distance(missile->EndPos()) < 200 && myhero->E()->isReady()) {
		auto target = targetselector->GetTargetByIndex(missile->SourceIndex());

		//	if (target != nullptr && myhero->inRange(target, myhero->E()->CastRange())) {
			//	myhero->E()->Cast(target);
		//	}
	}*/
	//if (missile->LineWidth() > 0)
	//	std::cout << "Missile: " << missile->Team() << " : " << missile->Name() << std::endl;
}

/*
* BuffGain
*/
void BuffGain(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() != myhero->Index())
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
* Q_Logic
*/
void Q_Logic(std::shared_ptr<sdk::GameObjectInstance> target) {
	
	int q_level = myhero->Q()->Level() - 1;
	float baseAttackRange = myhero->AttackRange();
	auto has_rocket_up = myhero->hasBuff(buff, "jinxq");
	float rocketRange = baseAttackRange + q_rocket_bonus_range[q_level];

	if ( has_rocket_up && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), baseAttackRange - q_rocket_bonus_range[q_level] - 50.0f)) {
		myhero->Q()->Cast();
	}
	else if ( !has_rocket_up && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), rocketRange) && !myhero->inRange(target.get(), baseAttackRange)) {
		myhero->Q()->Cast();
	}
}
/*
* W_logic
*/
void W_Logic(std::shared_ptr<sdk::GameObjectInstance> target) {


	auto bonus_as = myhero->BonusAttackSpeed();

	if (bonus_as > 2.5)
		bonus_as = 2.5;

	auto delay = 0.6 - ( 0.02 * static_cast<int>((bonus_as / 0.25)));

	auto prediction = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->W()->MissileSpeed(), myhero->W()->CastRange(), delay);

	if (prediction.isValid()) {
  	
		auto collide = predictionmanager->WillCollide(myhero->Position(), prediction, myhero->W()->LineWidth() + 30.0f, target.get(), true, true);
		if (!collide)
			myhero->W()->Cast(prediction);
	}
}

/*
* E_Logic
*/
void E_Logic()
{
	for(auto & target : targetselector->m_all_heros)
	{
		if (target->isAlly() || !target->isValid())
			continue;

		if (myhero->inRange(target.get(), myhero->E()->CastRange()) && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
			&& target->immobilized({ BuffTypes::Snare , BuffTypes::Stun , BuffTypes::Knockup , BuffTypes::Taunt , BuffTypes::Suppression }))
		{
			auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->E()->MissileSpeed(), myhero->E()->CastRange(), myhero->E()->CastDelay()/1000.0f);

			if (pos.isValid())
				myhero->E()->Cast(pos);
		
		}
	}
}



float calculateAverageSpeed(float travelDistance) {
	const float speed1 = 1700.0f;
	const float speed2 = 2200.0f;
	const float distance1 = 1350.0f;

	if (travelDistance <= distance1) {
		return speed1;
	}

	const float distance2 = travelDistance - distance1;
	const float time1 = distance1 / speed1;
	const float time2 = distance2 / speed2;
	const float totalTime = time1 + time2;
	const float averageSpeed = travelDistance / totalTime;

	return averageSpeed;
}

/*
* R_Killsteal
*/
void R_Killsteal()
{
	for (auto& target : targetselector->m_all_heros)
	{
		if (target->isAlly() || !target->isValid())
			continue;

		if (myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts() 
			&& myhero->inRange(target.get(), min_r_distance) 
			&& !myhero->inRange(target.get(), myhero->AttackRange()))
		{
			float rdamage = (r_damage[myhero->R()->Level() - 1] + (1.5 * myhero->BonusAttackDamage())) + (r_bonus[myhero->R()->Level() - 1] * (target->MaxHealth() - target->CurrentHealth()));
			auto overalldmg = myhero->TotalOverallDamage(target.get(), rdamage, 0.0f);
			
			if (target->CurrentHealth() < overalldmg) {

				const auto distance = myhero->Position().distance(target->aiServerPosition());
				const auto speed = calculateAverageSpeed(distance);

				 auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), speed, myhero->R()->CastRange(), myhero->R()->CastDelay()/1000.0f);
				if (pos.isValid()) {
					auto collide = predictionmanager->WillCollide(myhero->Position(), pos, myhero->R()->LineWidth() + 20.0f, target.get(), true, false);
					if(!collide)
						myhero->R()->Cast(pos);
				}
			}
		}
	}
}


/*
* Combo
*/
void Combo() {

	if (myhero->CurrentMana() < 25.0f || myhero->isCasting()) // min mana for Q
		return;

	// do the R kill still
//	if (settings->combo.use_semi_r) {
		R_Killsteal();
	//}

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 2000.0f);

	if (target == nullptr)
		return;

	auto curr_percent_mana = (myhero->CurrentMana() / myhero->MaxMana() * 100.0f);

	if(debug)
		std::cout << "mana percent: "  << curr_percent_mana << "q_mana" << q_mana << std::endl;

	float rdamage = (r_damage[myhero->R()->Level() - 1] + (1.5 * myhero->BonusAttackDamage())) + (r_bonus[myhero->R()->Level() - 1] * (target->MaxHealth() - target->CurrentHealth()));
	auto overalldmg = myhero->TotalOverallDamage(target.get(), rdamage, 0.0f);

	if (settings->combo.usee && myhero->CurrentMana() >= myhero->E()->GetManaCosts() && myhero->E()->isReady())
		E_Logic();

  	if (curr_percent_mana >= q_mana && settings->combo.useq && myhero->Q()->isReady()
		&& myhero->CurrentMana() >= myhero->Q()->GetManaCosts()) {
		Q_Logic(target);
	}
		
	if (settings->combo.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->W()->CastRange())
		&& !myhero->inRange(target.get(), myhero->AttackRange()+100.0f)) {

		W_Logic(target);
	}	

	if (settings->combo.user && myhero->R()->isReady() && myhero->inRange(target.get(), min_r_distance)) {
		// if myhero AA damgage*6 its less that the R damage use R 
		auto AAdmg = myhero->TotalOverallDamage(target.get(), myhero->TotalRawAttackDamage() * 6.0f, 0.0f);
		float rdamage = (r_damage[myhero->R()->Level() - 1] + (1.5 * myhero->BonusAttackDamage())) + (r_bonus[myhero->R()->Level() - 1] * (target->MaxHealth() - target->CurrentHealth()));
		auto overallRdmg = myhero->TotalOverallDamage(target.get(), rdamage, 0.0f);

		if (AAdmg > target->CurrentHealth() && target->CurrentHealth() < overallRdmg
			&& myhero->CurrentMana() >= myhero->R()->GetManaCosts()) {

			const auto distance = myhero->Position().distance(target->aiServerPosition());
			const auto speed = calculateAverageSpeed(distance);
			auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), 
				speed, myhero->R()->CastRange(), myhero->R()->CastDelay() / 1000.0f);
		

			if (pos.isValid() && target->isAlive()) {
				auto collide = predictionmanager->WillCollide(myhero->Position(), pos,
					myhero->R()->LineWidth() + 20.0f, target.get(), true, false);
				if (!collide)
					myhero->R()->Cast(pos);
			}
		}
	}
}

/*
 * LastHit
 */
void LastHit() {

	auto wdmg = w_damage[myhero->W()->Level() - 1] + (myhero->TotalRawAttackDamage() * 1.6f);
	auto wrange = myhero->W()->CastRange();
	auto target = targetselector->GetLastHitTarget(wrange, wdmg);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->laneclear.usew && myhero->W()->isReady() && myhero->inRange(target.get(), wrange)) {
		W_Logic(target);
	}
}

/*
 * Herass
 */
void Herass() {

	LastHit();

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->W()->CastRange());

	if (target == nullptr)
		return;


	if (settings->herass.useq) {
		for (auto& unit : targetselector->m_all_units) {

			if (!unit.second->isValid() || unit.second->isAlly())
				continue;

			Q_Logic(target);
		}		
	}

	if (settings->herass.usew && myhero->W()->isReady() && myhero->inRange(target.get(), myhero->W()->CastRange())) {
		W_Logic(target);
	}
}

/*
 * LaneClear
 */
void LaneClear() {

	auto wdmg = w_damage[myhero->W()->Level() - 1] + (myhero->TotalRawAttackDamage() * 1.6f);

	auto target = targetselector->GetLaneClearTarget(1000.0f, wdmg);

	// if i come from herass or laneclear or combo with rocket change to minigun. 
	auto has_rocket_up = myhero->hasBuff(buff, "jinxq");
	if (has_rocket_up && myhero->Q()->isReady())
		myhero->Q()->Cast();

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;
		
	if (settings->laneclear.usew && myhero->W()->isReady()  && myhero->inRange(target.get(), myhero->W()->CastRange())) {
		W_Logic(target);
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
			drawmanager->DrawCircle3D(myhero->Position(), min_r_distance, false, 60, RGBA_COLOR(150, 100, 100, 255), 2);
		}

		// DEBUG
		if (debug) {
			auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 2000.0f);

			if (target == nullptr)
				return;

			auto bonus_as = myhero->BonusAttackSpeed();

			if (bonus_as > 2.5)
				bonus_as = 2.5;

			auto delay = 0.6 - (0.02 * static_cast<int>((bonus_as / 0.25)));
			
			auto w_pred = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->R()->MissileSpeed(), myhero->R()->CastRange(), delay);

			auto collide = predictionmanager->WillCollide(myhero->Position(), w_pred, myhero->W()->LineWidth() + 30.0f, target.get(), true, true);

			if(w_pred.isValid())
				drawmanager->DrawCircle3D(w_pred, 30, true, 20, RGBA_COLOR(0, 222, 120, 255), 2);
		}
	
}

/*
* Menu
*/
void Menu() {
	auto root_node = menu->CreateSubmenu("CoreJinx");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddSliderFloat("Q min Mana", q_mana, 0.0f, 100.0f, "Minimum mana percent to use Q",true);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E on immobilized", &settings->combo.usee);
	sub_combo->AddCheckbox("Use E On GapCloser", &e_gap_closer);
	sub_combo->AddCheckbox("Use R to Killsteal", &settings->combo.user, "", true);
	sub_combo->AddSliderFloat("Min. R Distance", min_r_distance, 2000.0f, 50000.0f, "Minimun distance for Cast R to Target", true);

	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use W", &settings->laneclear.usew);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use W", &settings->lasthit.usew);

	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
* onLoad
*/
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->Q()->RegisterSpell(SkillAimType::activate);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target,0.6,1500.0f,0.0f,3300.0f);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target,0.6,5000.0f,0.0f);
	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	min_r_distance = settings->LoadCustomFloat("Min. R Distance")>0.0f? settings->LoadCustomFloat("Min. R Distance") : min_r_distance;
	q_mana = settings->LoadCustomFloat("Q min Mana")>0.0f ? settings->LoadCustomFloat("Q min Mana") : q_mana;

	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");
	e_gap_closer = settings->LoadCustomBool("Use E On GapCloser");

	Menu();

	event_handler<Events::onUpdate>::add_callback(Update);
	event_handler<Events::onBeforeAttack>::add_callback(BeforeAttack);
	event_handler<Events::onAfterAttack>::add_callback(AfterAttack);
	event_handler<Events::onSpellCast>::add_callback(SpellCast);
	event_handler<Events::onMissileCreate>::add_callback(MissileCreate);
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

	event_handler<Events::onUpdate>::remove_handler(Update);
	event_handler<Events::onBeforeAttack>::remove_handler(BeforeAttack);
	event_handler<Events::onAfterAttack>::remove_handler(AfterAttack);
	event_handler<Events::onSpellCast>::remove_handler(SpellCast);
	event_handler<Events::onMissileCreate>::remove_handler(MissileCreate);
	event_handler<Events::onGainBuff>::remove_handler(BuffGain);
	event_handler<Events::onLoseBuff>::remove_handler(BuffLose);
	event_handler<Events::onGapCloser>::remove_handler(GapCloser);
	event_handler<Events::onDraw>::remove_handler(Draw);
}