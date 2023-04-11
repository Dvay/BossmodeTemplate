 #include "pch.h"
#include <string>
using namespace sdk;

static std::string championName = "CoreActivator";
static std::string championVersion = "0.0.8";

CHAMPIONS("");
PLUGIN_NAME("CoreActivator");
PLUGIN_TYPE(PluginTypes::UTILS);

static std::shared_ptr<BuffInstance> buff;
static std::shared_ptr<SpellInstance> cleanse_summoner;
static std::shared_ptr<SpellInstance> smite_summoner;
static std::shared_ptr<SpellInstance> heal_summoner;
static std::shared_ptr<SpellInstance> barrier_summoner;
static std::shared_ptr<SpellInstance> ignite_summoner;
static std::shared_ptr<SpellInstance> exhaust_summoner;

static float last_time_cc = 0.0;
static bool item_galeforce = false;
static int toggle_smite_key = 0x4A; // (J)

static bool script_detector = false;
static bool jungle_alert = true;
static bool show_close_enemys = true;


static float galeforce_dmg[10] = { 270.0, 292.5, 315, 337.5, 360.0, 382.5, 405.0, 427.5, 450.0, 472.5 };
/*
 * BeforeAttack
 */
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;

	auto pos = target->Position();
	
	// Ignite
	if (ignite_summoner != nullptr && settings->summoners.use_ignite && ignite_summoner->isReady()
		&& target->CurrentHealth() < ignite_summoner->Damage()) {

		ignite_summoner->Cast(pos);
	}
	// Exaust
	else if (exhaust_summoner != nullptr && settings->summoners.use_exhaust && exhaust_summoner->isReady()
		&& settings->summoners.exhaust_health >= (target->CurrentHealth() / target->MaxHealth())) {
		exhaust_summoner->Cast(pos);
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

/*
 * MissileCreate
 */
void MissileCreate(std::shared_ptr < MissileInstance> missile) { }

/*
 * MissileRemove
 */
void MissileRemove(std::shared_ptr < MissileInstance> missile) { }

/*
 * BuffGain
 */
void BuffGain(std::shared_ptr<BuffInstance> buff) {

//	std::cout << buff->Name() << std::endl;
//	if (buff->SourceIndex() == myhero->Index())

	//	return;
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
*
*/
void Potions_logic() 
{
	// Auto Potions managments
	static float next_potion_usage_time = 0;
	if (settings->potions.enabled && myhero->isAlive()
		&& ((myhero->CurrentHealth() / myhero->MaxHealth()) * 100.0f) <= settings->potions.use_potion_percentage && game->GameTime() > next_potion_usage_time) {
		std::shared_ptr<ItemInstance> potion = nullptr;
		
		//	std::cout << "hero healt %:  " << (yhero->CurrentHealth() / myhero->MaxHealth()) * 100.0f << std::endl;
		//	std::cout << "cast on %:     " << settings->potions.use_potion_percentage << std::endl;
		
		// 2003 => Health pot		
		if (settings->potions.use_healt_potion) {
			int slot = myhero->GetItemSlot(2003);
			potion = myhero->GetItemBySlot(slot);
		}
		// 2010 => Total Biscuit 
		if (!potion && settings->potions.use_biscuit_cookie) {
			int slot = myhero->GetItemSlot(2010);
			potion = myhero->GetItemBySlot(slot);
		}
		// 2031 Refillable Potion	
		if (!potion && settings->potions.use_refillable_potion) {
			int slot = myhero->GetItemSlot(2031);
			potion = myhero->GetItemBySlot(slot);
		}
		// 2033 => Corrupting Potion	
		if (!potion && settings->potions.use_corrupting_potion) {
			int slot = myhero->GetItemSlot(2033);
			potion = myhero->GetItemBySlot(slot);
		}

		// cast item depending if i have it or not
		if (potion && potion->isReady() > 0) {
			potion->Cast();
			next_potion_usage_time = game->GameTime() + 15000.0f;  // 15 seconds delay before using the next one
		}
	}
}

/*
* GaleForce Logic
*/
void Galeforce_Logic() {
		
	
	// 6671 galeforce ID
	std::shared_ptr<ItemInstance> galeforce = nullptr;
	int slot = myhero->GetItemSlot(6671);
	galeforce = myhero->GetItemBySlot(slot);

	auto lvl = myhero->Level();
	if (lvl > 9)
		lvl -= 9;
	else
		lvl = 0;


	if (galeforce && galeforce->isReady()) {
		
		auto ap = galeforce_dmg[lvl] + (0.675 * myhero->BonusAttackDamage());
		for (auto& target : targetselector->m_all_heros) {

			if (!target->isValid() || target->isAlly())
				continue;

		auto dmg = myhero->TotalOverallDamage(target.get(), 0.0, ap);


		if(target->CurrentHealth() < dmg)
			galeforce->Cast(target->Position());

		}
	}
}

/*
 * Cleanse_logic
 */
void Cleanse_logic()
{
	static float last_casted = 0;
	if (!settings->QSS.enabled)
		return;

	auto game_time = game->GameTime();
	if (myhero->immobilized() && last_time_cc == 0.0f)
	{
		last_time_cc = game->GameTime() + settings->QSS.delay;
		return;
	}

	if( last_time_cc > 0.0f && last_time_cc <= game_time)
	{
		last_time_cc = 0.0f;

		if (settings->QSS.use_cleanse && cleanse_summoner != nullptr && last_casted +1000.0f < game_time && cleanse_summoner->isReady()) {
			cleanse_summoner->Cast();
			last_casted = game_time;
		}
		else if (settings->QSS.use_item && last_casted + 1000.0f < game_time) {
			bool casted = false;
			auto qss_item = myhero->GetItemSlot(3140); // qss

			if (qss_item > 0)
			{
				auto cast_item = myhero->GetItemBySlot(qss_item);
				if (cast_item->isReady()) {
					cast_item->Cast();
					casted = true;
					last_casted = game_time;
				}
			}

			if (!casted) {
				auto mercurial = myhero->GetItemSlot(3139); // ItemMercurial

				if (mercurial > 0)
				{
					auto cast_item = myhero->GetItemBySlot(mercurial);
					if (cast_item->isReady()) {
						cast_item->Cast();
						casted = true;
						last_casted = game_time;
					}
				}
			}

			if (!casted) {
				auto silvermere_dawn = myhero->GetItemSlot(6035); // ItemMercurial

				if (silvermere_dawn > 0)
				{
					auto cast_item = myhero->GetItemBySlot(silvermere_dawn);
					if (cast_item->isReady()) {
						cast_item->Cast();
						casted = true;
						last_casted = game_time;
					}
				}
			}
		}
	}
}


void AutoSmite() {

	if (smite_summoner != nullptr  && smite_summoner->isReady()) {

		for (auto& unit : targetselector->m_all_units) {

			if (!unit.second->isValid() || unit.second->isAlly() || !myhero->inRange(unit.second.get(), smite_summoner->CastRange()) 
				|| ((!unit.second->is_dragon() || (!settings->smitable_objects["Dragon"] && !settings->smitable_objects["All"])) && 
					(!unit.second->is_baron() || (!settings->smitable_objects["Baron"] && !settings->smitable_objects["All"])) &&
					(!unit.second->is_herald() || (!settings->smitable_objects["Herald"] && !settings->smitable_objects["All"])) &&
					(!unit.second->is_blue() || (!settings->smitable_objects["Blue"] && !settings->smitable_objects["All"])) &&
					(!unit.second->is_red() || (!settings->smitable_objects["Red"] && !settings->smitable_objects["All"])) &&
					(!unit.second->is_crab() || (!settings->smitable_objects["Crab"] && !settings->smitable_objects["All"]))))
				continue;

			
		
			if (unit.second->CurrentHealth() <= smite_summoner->Damage())
				smite_summoner->Cast(unit.second);
		}
	}
}
/*
 * Combo
 */
void Combo() {

	// Cleanse Logic
	Cleanse_logic();

	// GaleForce
	if(item_galeforce)
		Galeforce_Logic();

	// Heal
	if (heal_summoner != nullptr && settings->summoners.use_heal && heal_summoner->isReady() 
		&& settings->summoners.heal_barrier_health >= (myhero->CurrentHealth() / myhero->MaxHealth() * 100.0f)) {
		heal_summoner->Cast();
	}
	// Barrier
	else if (barrier_summoner != nullptr && settings->summoners.use_barrier && barrier_summoner->isReady()
		&& settings->summoners.heal_barrier_health >= (myhero->CurrentHealth() / myhero->MaxHealth() * 100.0f)) {
		barrier_summoner->Cast();
	}
}

/*
 * Update
 */
void Update() {


	if ((GetAsyncKeyState(toggle_smite_key) >> 16) & 1) {

		if (settings->summoners.use_smite)
			settings->summoners.use_smite = false;
		else
			settings->summoners.use_smite = true;

		settings->SaveCore();

		while (GetAsyncKeyState(toggle_smite_key))
			Sleep(1);
	}

	// AutoSmite
	if(settings->summoners.use_smite)
		AutoSmite();
	
	// Potions
	if (orbwalker->Mode() != OrbwalkModes::NONE) {
		Potions_logic();
	}

	// Combo
	if (orbwalker->Mode() == OrbwalkModes::COMBO) {
		Combo();
	}
}


float x_offset = 39.78;
float y_offset = 8.0;
float w_offset = 32.0;
float h_offset = 4.0;
/*
 * Draw
 */
void Draw() {
	

		
		auto mypos = gamerenderer->WorldToScreen(myhero->Position());
		
		if (settings->summoners.draw_smite_status) {
			if (settings->summoners.use_smite) {
				drawmanager->DrawText("Smite On", 20.0f, mypos, RGBA_COLOR(0, 255, 0, 255));
			}
			else
				drawmanager->DrawText("Smite Off", 20.0f, mypos, RGBA_COLOR(255, 0, 0, 255));


			mypos.y += 22.0f;
		}



		if (settings->draw_evade_state) {
			if (settings->enable_evading) {

				drawmanager->DrawText("Evade On", 20.0f, mypos, RGBA_COLOR(0, 255, 0, 255));
			}
			else {

				drawmanager->DrawText("Evade Off", 20.0f, mypos, RGBA_COLOR(255, 0, 0, 255));

			}

		}


		if (script_detector || jungle_alert || show_close_enemys) {

			 auto mypos = myhero->Position();
			 int jungler_id = 0;
			
			 bool close_enemy_present = false;
			for (auto& hero : targetselector->m_all_heros) {

				if (script_detector) {
					if (hero->GetAverageClickSpeed() < 170.0 && hero->GetAverageClickSpeed() > 0.0f) {

						 auto pos = gamerenderer->WorldToScreen(hero->Position());

						pos.y -= 40.0f;
						drawmanager->DrawText("Probably Scripting", 20.0f, pos, RGBA_COLOR(123, 255, 180, 255));
					}
				}

				const auto hero_pos = hero->Position();
				if (jungle_alert && hero->isValid() && !hero->isAlly() && myhero->inRange(hero.get(), 3000.0f) && (hero->SUM1()->SpellName().find("SummonerSmite") != std::string::npos || hero->SUM2()->SpellName().find("SummonerSmite") != std::string::npos)) {
					
					const auto image = hero->GetImage(true);
					
				
					const auto extend = mypos.extend(hero_pos, 300);
					drawmanager->DrawLine(mypos, hero_pos, 4.0, sdk::RGBA_COLOR(255, 0, 0));
					drawmanager->DrawCircularImage(image, extend, 50.0f);

					jungler_id = hero->NetworkId();
				}

				if (show_close_enemys && hero->isValid() && !hero->isAlly() && myhero->inRange(hero.get(), 3000.0f) && !gamerenderer->IsWorldPointOnScreen(hero_pos))
					{
					if (jungle_alert && jungler_id == hero->NetworkId())
						continue;

							const auto distance = mypos.distance(hero_pos);
							const auto multiplier = distance / 3000.0f;
							const float size = 60.0f;
							const auto show_range = 700.0f * multiplier;
							const auto image = hero->GetImage(true);
							const auto extend = mypos.extend(hero_pos, show_range);
							
							drawmanager->DrawCircle2D(extend, size/2.0+2.0f,true,35, sdk::RGBA_COLOR(255,0,0),2);
							drawmanager->DrawCircularImage(image, extend, size);

							close_enemy_present = true;
					}
			}

			if (show_close_enemys && close_enemy_present)
				drawmanager->DrawCircle3D(mypos, 700, false, 35, sdk::RGBA_COLOR(155, 50, 60, 80), 2);
		}

	


	/*	for (auto& unit : targetselector->m_all_units) {

			if (!unit.second->isValid() || unit.second->isAlly() || !myhero->inRange(unit.second.get(), myhero->AttackRange() + 200.0f))
				continue;


			auto pos = unit.second->Position();
			pos.y += y_offset;

			auto screen = gamerenderer->WorldToScreen(pos);

			screen.y -= (gamerenderer->ScreenHeight() * 0.00083333335f * pos.y);
			auto screen_left = Vector2(screen.x - x_offset, screen.y);
			auto screen_right = Vector2(screen.x + w_offset, screen.y + h_offset);
			drawmanager->DrawRect2D(screen_left, screen_right,true, RGBA_COLOR(0, 255, 0));
			
		}*/
}

/*
 * Menu
 */
void Menu() {
	auto root_node = menu->CreateSubmenu("CoreActivator");

	auto sub_potions = menu->CreateSubmenu("Potions");
	auto sub_smite = menu->CreateSubmenu("Smite");
	sub_potions->AddCheckbox("Enabled", &settings->potions.enabled);
	sub_potions->AddCheckbox("Use Red", &settings->potions.use_healt_potion);
	sub_potions->AddCheckbox("Use Biscuit", &settings->potions.use_biscuit_cookie);
	sub_potions->AddCheckbox("Use Refillable", &settings->potions.use_refillable_potion);
	sub_potions->AddCheckbox("Use Corrupting", &settings->potions.use_corrupting_potion);
	sub_potions->AddSliderFloat("Potions on Health %", settings->potions.use_potion_percentage, 0.0f, 100.0f,
		"Will Use Potions Depending your current healt");

	auto sub_summoners = menu->CreateSubmenu("Summoners");
	sub_summoners->AddCheckbox("Use Heal", &settings->summoners.use_heal);
	sub_summoners->AddCheckbox("Use Barrier", &settings->summoners.use_barrier);
	sub_summoners->AddSliderFloat("Heal/Barrier on Health %", settings->summoners.heal_barrier_health, 0.0f, 100.0f, 
		"Will Use Summoner Heal or Barrier" );

	sub_summoners->AddCheckbox("Use Exhaust", &settings->summoners.use_exhaust);
	sub_summoners->AddSliderFloat("Exhaust on Health %", settings->summoners.exhaust_health, 0.0f, 100.0f, 
		"Will Use Summoner Heal or Barrier");

	sub_summoners->AddCheckbox("Use Igniter", &settings->summoners.use_ignite);
	sub_smite->AddCheckbox("Enable", &settings->summoners.use_smite);
	sub_smite->AddCheckbox("Draw status", &settings->summoners.draw_smite_status);
	auto sub_smite_units = menu->CreateCollapsible("Units");

	sub_smite_units->AddCheckbox("All",	   &settings->smitable_objects["All"]);
	sub_smite_units->AddCheckbox("Baron",  &settings->smitable_objects["Baron"]);
	sub_smite_units->AddCheckbox("Herald", &settings->smitable_objects["Herald"]);
	sub_smite_units->AddCheckbox("Dragon", &settings->smitable_objects["Dragon"]);
	sub_smite_units->AddCheckbox("Red",    &settings->smitable_objects["Red"]);
	sub_smite_units->AddCheckbox("Blue",   &settings->smitable_objects["Blue"]);
	sub_smite_units->AddCheckbox("Crab",   &settings->smitable_objects["Crab"]);
	sub_smite->addItem(sub_smite_units);
	sub_smite->AddHotkey("Toggle Smite", toggle_smite_key, true, "will turn auto smite on or off");

	sub_summoners->addItem(sub_smite);

	auto sub_qss = menu->CreateSubmenu("QSS/Cleanse");
	sub_qss->AddCheckbox("Enable", &settings->QSS.enabled);
	sub_qss->AddCheckbox("Use Summoner Cleanse", &settings->QSS.use_cleanse);
	sub_qss->AddCheckbox("Use Item (QSS)", &settings->QSS.use_item);
	sub_qss->AddSliderFloat("Delay", settings->QSS.delay, 0.0f, 700.0f, 
		"Will use cast the item delayed by the amount in milliseconds");
	auto sub_qss_on = menu->CreateSubmenu("On CCs");

	auto sub_items = menu->CreateSubmenu("Items");
	auto sub_items_adc = menu->CreateSubmenu("ADCs");
		sub_items_adc->AddCheckbox("Use GaleForce", &item_galeforce);
	sub_items->addItem(sub_items_adc);

	auto sub_misc = menu->CreateSubmenu("Misc");
	sub_misc->AddCheckbox("Show other scripters", &script_detector,"Shows who is scripting most likely due to movement behaviour");

	auto sub_alerts = menu->CreateSubmenu("Gank Alert");
	sub_alerts->AddCheckbox("Show Jungler Alert", &jungle_alert, "Shows when a jungler is close", true, "core_activator");
	sub_alerts->AddCheckbox("Show close enemy images", &show_close_enemys, "Shows the image of enemys which are close to you on the screen",true,"core_activator");
	/*sub_misc->AddSliderFloat("x offset", x_offset, 0.0f, 500.0f);
	sub_misc->AddSliderFloat("y offset", y_offset, 0.0f, 500.0f);
	sub_misc->AddSliderFloat("w offset", w_offset, 0.0f, 500.0f);
	sub_misc->AddSliderFloat("h offset", h_offset, 0.0f, 500.0f);*/


	
	for (auto& val : settings->QSS.immobilizations) {
		sub_qss_on->AddCheckbox(val.second.first, &val.second.second);
	}

	sub_qss->addItem(sub_qss_on);

	root_node->addItems({ sub_potions, sub_summoners, sub_qss, sub_items,sub_misc,sub_alerts });

	menu->addItem(root_node);
}

/*
 * onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);
	
	/*
	exhaust_health = 61.537998
	heal_on = 22.222000
	use_potion_percentage = 47.058998
	*/
	if (settings->QSS.delay <= 0.0f)
		settings->QSS.delay = 42.000000;

	if (settings->summoners.exhaust_health <= 0.0f)
		settings->summoners.exhaust_health = 61.537998;

	if (settings->summoners.heal_barrier_health <= 0.0f)
		settings->summoners.heal_barrier_health = 22.222444;

	if (settings->potions.use_potion_percentage <= 0.0f)
		settings->potions.use_potion_percentage = 47.058998;

	if (myhero->SUM1()->SpellName() == "SummonerBoost") {
		cleanse_summoner = myhero->SUM1();
	}
	else if (myhero->SUM2()->SpellName() == "SummonerBoost") {
		cleanse_summoner = myhero->SUM2();
	}

	if (myhero->SUM1()->SpellName() == "SummonerHeal")
		heal_summoner = myhero->SUM1();
	else if (myhero->SUM2()->SpellName() == "SummonerHeal")
		heal_summoner = myhero->SUM2();


	if (myhero->SUM1()->SpellName() == "SummonerBarrier")
		barrier_summoner = myhero->SUM1();
	else if (myhero->SUM2()->SpellName() == "SummonerBarrier")
		barrier_summoner = myhero->SUM2();


	if (myhero->SUM1()->SpellName().find("SummonerSmite") != std::string::npos) {
		myhero->SUM1()->RegisterSpell(SkillAimType::direction_to_target);
		smite_summoner = myhero->SUM1();
	}
	else if (myhero->SUM2()->SpellName().find("SummonerSmite") != std::string::npos) {
		myhero->SUM2()->RegisterSpell(SkillAimType::direction_to_target);
		smite_summoner = myhero->SUM2();
	}

	if (myhero->SUM1()->SpellName() == "SummonerExhaust") {
		myhero->SUM1()->RegisterSpell(SkillAimType::direction_to_target);
		exhaust_summoner = myhero->SUM1();
	}
	else if (myhero->SUM2()->SpellName() == "SummonerExhaust") {
		myhero->SUM2()->RegisterSpell(SkillAimType::direction_to_target);
		exhaust_summoner = myhero->SUM2();
	}

	if (myhero->SUM1()->SpellName() == "SummonerDot") {
		myhero->SUM1()->RegisterSpell(SkillAimType::direction_to_target);
		ignite_summoner = myhero->SUM1();
	
	}
	else if (myhero->SUM2()->SpellName() == "SummonerDot") {
		myhero->SUM2()->RegisterSpell(SkillAimType::direction_to_target);
		ignite_summoner = myhero->SUM2();
	
	}


	if(!settings->LoadCustomString("Show Jungler Alert", "core_activator").empty())
	jungle_alert = std::stoi(settings->LoadCustomString("Show Jungler Alert", "core_activator"));

	if (!settings->LoadCustomString("Show close enemy images", "core_activator").empty())
	show_close_enemys = std::stoi(settings->LoadCustomString("Show close enemy images", "core_activator"));

	if (!settings->LoadCustomString("Show other scripters", "core_activator").empty())
	script_detector = std::stoi(settings->LoadCustomString("Show other scripters", "core_activator"));

	Menu();

	//event_handler<Events::onCreateObject>::add_callback(CreateObject);
	//event_handler<Events::onDeleteObject>::add_callback(DeleteObject);

	event_handler<Events::onUpdate>::add_callback(Update);

	event_handler<Events::onBeforeAttack>::add_callback(BeforeAttack);
	event_handler<Events::onAfterAttack>::add_callback(AfterAttack);
	/*event_handler<Events::onSpellCast>::add_callback(SpellCast);

	event_handler<Events::onMissileCreate>::add_callback(MissileCreate);
	event_handler<Events::onMissileRemoved>::add_callback(MissileRemove);*/

	event_handler<Events::onGainBuff>::add_callback(BuffGain);
	event_handler<Events::onLoseBuff>::add_callback(BuffLose);

	//event_handler<Events::onGapCloser>::add_callback(GapCloser);

	event_handler<Events::onDraw>::add_callback(Draw);
}

/*
 * onUnload
 */
void onUnload() {

	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " unLoaded" << std::endl;

	//event_handler<Events::onCreateObject>::remove_handler(CreateObject);
	//event_handler<Events::onDeleteObject>::add_callback(DeleteObject);

	event_handler<Events::onUpdate>::remove_handler(Update);

	event_handler<Events::onBeforeAttack>::remove_handler(BeforeAttack);
	event_handler<Events::onAfterAttack>::remove_handler(AfterAttack);
	/*event_handler<Events::onSpellCast>::remove_handler(SpellCast);

	event_handler<Events::onMissileCreate>::remove_handler(MissileCreate);
	event_handler<Events::onMissileRemoved>::remove_handler(MissileRemove);
	*/
	event_handler<Events::onGainBuff>::remove_handler(BuffGain);
	event_handler<Events::onLoseBuff>::remove_handler(BuffLose);

//	event_handler<Events::onGapCloser>::remove_handler(GapCloser);

	event_handler<Events::onDraw>::remove_handler(Draw);
}