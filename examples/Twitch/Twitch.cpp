#include "pch.h"
using namespace sdk;

static std::string championName = "Twitch";
static std::string championVersion = "0.0.3";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreTwitch");
PLUGIN_TYPE(PluginTypes::CHAMPION);

static std::shared_ptr<BuffInstance> buff;
bool debug = false;

static float rawDmg[5] = { 20,30,40,50,60 };
static float rawDmgPerStack[5] = { 15, 20, 25, 30, 35 };

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;

// Misc checks
bool e_to_killsteal = true;
int min_targets_to_hit = 2;
float min_r_health = 450.0f;
bool e_on_max_stacks = true;
bool do_q_on_recall = false;
Vector3 pred_pos;


float eDamage(GameObjectInstance* target) {
	float total_dmg = 0.0f;
	/*
		PhysicalDamageMod by get_real_dagame and FlatMagicDamageMod get_ability_power
		BASE PHYSICAL DAMAGE: 20 / 30 / 40 / 50 / 60
		BONUS DAMAGE PER STACK: 15 / 20 / 25 / 30 / 35 (+ 35% bonus AD)
	*/

	//dmgperstacks = (dmgperstacks) / (1 + (target->get_armor() / 100.0));

	float ad_dmg = (rawDmg[myhero->E()->Level() - 1] + (((rawDmgPerStack[myhero->E()->Level() - 1] +
		(myhero->BonusAttackDamage() * 0.35f)) * buff->MaxStackCount())));
	float ap_dmg = ((myhero->AbilityPower() * 0.35f) * buff->MaxStackCount());

	total_dmg = myhero->TotalOverallDamage(target, ad_dmg, ap_dmg) ;

	//std::cout << rawDmg[myhero->E()->Level() - 1] << ":" << rawDmgPerStack[myhero->E()->Level() - 1]  << ":" << myhero->BonusAttackDamage()  << ":" << buff->MaxStackCount()  << ":" << ad_dmg << ":ap: " << ap_dmg<< ":total: " << total_dmg << std::endl;

	return total_dmg;
}

/*
* BeforeAttack
*/
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;
	
	if (orbwalker->Mode() == OrbwalkModes::COMBO) {

		if (settings->combo.usee && e_on_max_stacks && (myhero->CurrentMana() > myhero->E()->GetManaCosts() && myhero->E()->isReady())
			&& target->hasBuff(buff, "twitchdeadlyvenom"))
		{
			// Debug E Damage
			if (debug) {
				std::cout <<
					" Target: " << target->CurrentMana() <<
					" E-LVl: " << myhero->E()->Level() <<
					" E-Ready: " << myhero->E()->isReady() <<
					" Stack Count: " << buff->StackCount() <<
					" Damage to do: " << eDamage(target) <<
					" Target health: " << target->CurrentHealth() <<
					" in range E:" << myhero->inRange(target, myhero->E()->CastRange())
					<< std::endl;
			}

			
			if (buff->StackCount() > 5) {
					myhero->E()->Cast();
				}
			
		}
	}
	
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

//static std::shared_ptr<MissileInstance> r_missile;
/*
* MissileCreate
*/
void MissileCreate(std::shared_ptr<MissileInstance> missile) {


	// used r_missile to draw Twitch R , as i wanted to check collision check might work
	/*if (missile->Name() == "TwitchSprayAndPrayAttack") {

		r_missile = missile;
		std::cout << missile->LineWidth() << std::endl;
		std::cout << missile->MissileRange() << std::endl;

	}*/
}

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

	//std::cout << buff->Name() << std::endl ;
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
void DeleteObject(std::shared_ptr<GameObjectInstance> obj) {}

/*
* KillStealE
*/
void KillStealE() {

	auto e_level = myhero->E()->Level();
	auto e_ready = myhero->E()->isReady();
	auto current_mana = myhero->CurrentMana();

	for (auto& t : targetselector->m_all_heros) {

		if (t->isAlly() || !t->isValid())
			continue;


	//	if(t->hasBuff(buff, "twitchdeadlyvenom"))
		//	std::cout <<  "eDamage:" << eDamage(t.get()) << "Enemy Health:" << t->CurrentHealth() << std::endl;

		if (e_ready && current_mana >= myhero->E()->GetManaCosts() && myhero->inRange(t.get(), myhero->E()->CastRange()) 
			&& t->hasBuff(buff, "twitchdeadlyvenom") && eDamage(t.get()) >= t->CurrentHealth()) {
			myhero->E()->Cast();
			return;
		}
	}
}

int countEnemysHitByR(GameObjectInstance* target) {
		Vector3 outHitPoint;
		int count = 0;
		auto start = myhero->Position();
		auto direction = target->Position().direction(start).normalize();
		auto end = start.add(direction.scale(1100.0));

		for (auto& t : targetselector->m_all_heros) {

			if (t->isAlly() || !t->isValid())
				continue;

			//WillCollide checks if a given target will collide with a rectangular missile of a given width, so i use that to check if a target 
			//is inside Twitch R missiles
			

			if (predictionmanager->isObjectOnMissilePath(outHitPoint, myhero->Position(), end, t.get(), 120.0f))
				count++;
		}
		return count;
}

/*
* Combo
*/
void Combo() {

	// e_to_killsteal
	if (settings->combo.usee && e_to_killsteal)
		KillStealE();

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 1200.0f);

	if (target == nullptr)
		return;

	//std::cout << countEnemysHitByR(target.get()) << std::endl;
	pred_pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->W()->MissileSpeed(), myhero->W()->CastRange() / 1000.0f, myhero->W()->CastDelay());

	// we need to add something to debug only champion like settings->debug_champions
	if (debug)
	{
		std::cout << " =================================================================================== " << std::endl;
		std::cout << " Target name: " << target->ChampionName() << std::endl;
		std::cout << " Target Healt: " << target->CurrentHealth() << std::endl;
		std::cout << " R isReady: " << myhero->R()->isReady() << std::endl;
		std::cout << " E isReady: " << myhero->E()->isReady() << std::endl;
		std::cout << " in_range R: " << myhero->inRange(target.get(), myhero->R()->CastRange()) << std::endl;
		std::cout << " Target position: " << target->Position().x << " : " << target->Position().y << std::endl;
		std::cout << " =================================================================================== "<< std::endl;
	}

	if (settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		myhero->Q()->Cast(target);
	}
	else if (settings->combo.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {

		 if (pred_pos.isValid()) {
			 myhero->W()->Cast(pred_pos);
			 //pred_pos.reset();
		 }
	}
	else if (settings->combo.user && myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->R()->CastRange()) && countEnemysHitByR(target.get()) >= min_targets_to_hit) {
		myhero->R()->Cast(target);
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

	auto target = targetselector->GetLaneClearTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(),myhero->AttackRange())))
		return;

	if (settings->laneclear.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {
		myhero->W()->Cast(target);
	}
	else if (settings->laneclear.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
			myhero->E()->Cast();
	}


}

/*
* LastHit
*/
void LastHit() { 

		auto target = targetselector->GetLastHitTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;


	if (settings->lasthit.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {
			myhero->W()->Cast(target);
	}
	else if (settings->lasthit.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
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
	} else 	if (orbwalker->Mode() == OrbwalkModes::NONE) {
		//  do_steatlth_back
		if (do_q_on_recall && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
			&& GetAsyncKeyState(0x42)) {
			if(debug)
				std::cout << "Steal back ON!" << std::endl;
			// you have to add a method for the B as i dont want to sendinput if you have another valid method to cast for keys.
			// if i press B hit B again to cancel my backing and press Q + B to start backing stealth mode.
			//myhero->B()->Cast();
			myhero->Q()->Cast();
			game->PressKey(0x42);
			//myhero->E()->Cast();
			//myhero->B()->Cast();
		}
	}
}

/*
* Draw
*/
void Draw() {
	
		if (!myhero->isAlive())
			return;

		/* used this to visualize Twitch R missiles	
		if (r_missile != nullptr) {
			auto direction = r_missile->EndPos().direction(r_missile->StartPos()).normalize();
			auto end = r_missile->StartPos().add(direction.scale(1100.0));
			drawmanager->DrawRect3D(r_missile->StartPos(), end, 120.0f , RGBA_COLOR(255, 0, 0, 255));
		}*/

		if(pred_pos.isValid())
			drawmanager->DrawCircle3D(pred_pos, 120, false, 40, RGBA_COLOR(135, 0, 100, 255), 2);

		if (draw_q_range && myhero->Q()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->AttackRange(), false, 60, RGBA_COLOR(135, 0, 100, 255), 2);
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
	auto root_node = menu->CreateSubmenu("CoreTwitch");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);	
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);


	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);

	auto sub_e = menu->CreateSubmenu("E usage");
	sub_e->AddCheckbox("Always E on max stacks", &e_on_max_stacks, {}, true);
	sub_e->AddCheckbox("E to Killsteal", &e_to_killsteal, {}, true);
	sub_combo->addItem(sub_e);

	sub_combo->AddCheckbox("Use R", &settings->combo.user);
	sub_combo->AddSliderFloat("Use R Healt <=", min_r_health, 100.0f, 2000.0f, {}, true);
	sub_combo->AddSliderInt("Min targets", min_targets_to_hit, 1, 5, {}, true);

	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);

	
	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use W", &settings->laneclear.usew);
	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);


	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use W", &settings->lasthit.usew);
	sub_lasthit->AddCheckbox("Use E", &settings->lasthit.usee);


	auto sub_misc = menu->CreateSubmenu("Misc");
	sub_misc->AddCheckbox("Use Q on Recall", &do_q_on_recall,
		"Press and hold B to recall while stealth mode. Only works when Q is ready", true);
	root_node->addItems({ sub_combo,  sub_herass, sub_laneclear, sub_lasthit,sub_misc, drawing_node });

	menu->addItem(root_node);
}

/*
* onLoad
*/
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;
	
	min_r_health = settings->LoadCustomFloat("Use R Healt <=") > 0.0f ? settings->LoadCustomFloat("Use R Healt <=") : min_r_health;
	min_targets_to_hit = settings->LoadCustomInt("Min targets") > 0 ? settings->LoadCustomInt("Min targets") : min_targets_to_hit;
	
	do_q_on_recall = settings->LoadCustomBool("Make Stealth Backing");
	e_to_killsteal = settings->LoadCustomBool("E to Killsteal");
	e_on_max_stacks = settings->LoadCustomBool("Always E on max stacks");
	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");

	Menu();

	myhero->Q()->RegisterSpell(SkillAimType::activate);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target,0.250,900.0f,120.0f,1400.0f,0.0f);
	myhero->E()->RegisterSpell(SkillAimType::activate);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

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