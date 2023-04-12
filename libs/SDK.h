#pragma once
#pragma warning (disable: 4273 4099 4251)
#include <Windows.h>
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <unordered_map>
#include <map>



#define PLUGIN_API	extern "C" __declspec(dllexport)

#ifdef EXPORTS_API
#define BUZZ_API_EXPORT __declspec (dllexport)
#else
#define BUZZ_API_EXPORT __declspec (dllimport)
#endif

#define buff_hash(str) (std::integral_constant<std::uint32_t, buff_hash_real(str)>::value)
#define spell_hash(str) (std::integral_constant<std::uint32_t, spell_hash_real(str)>::value)

constexpr std::uint32_t const buff_hash_real(const char* str)
{
	std::uint32_t hash = 0x811C9DC5;
	std::uint32_t len = 0;

	while (str[len] != '\0')
		len++;

	for (auto i = 0u; i < len; ++i)
	{
		char current = str[i];
		char current_upper = current + 0x20;

		if (static_cast<uint8_t>(current - 0x41) > 0x19u)
			current_upper = current;

		hash = 16777619 * (hash ^ current_upper);
	}

	return hash;
}

/**
 * Same as spell_hash but you can use it in runtime
 *
 * Example: spell_hash_real("ZeriR");
 */
constexpr std::uint32_t const spell_hash_real(const char* str) /*use for script_spell* name*/
{
	std::uint32_t hash = 0;
	std::uint32_t len = 0;

	while (str[len] != '\0')
		len++;

	for (auto i = 0u; i < len; ++i)
	{
		char current = str[i];
		char current_upper = current + 0x20;

		if (static_cast<uint8_t>(current - 0x41) > 0x19u)
			current_upper = current;

		hash = current_upper + 0x10 * hash;

		if (hash & 0xF0000000)
			hash ^= (hash & 0xF0000000) ^ ((hash & 0xF0000000) >> 24);
	}

	return hash;
}

namespace sdk {
	struct Vector2 {
		Vector2() {};
		Vector2(float _x, float _y) {
			x = _x;
			y = _y;
		}

		float x = 0.0f;
		float y = 0.0f;

		bool isValid() {
			return x > 0.0f && y > 0.0f  && x < 5000.0 && y < 5000.0;
		}
		
		float length() {
			return (float)sqrt(x * x + y * y);
		}

		float distance(const Vector2& o) {
			return (float)sqrt(pow(x - o.x, 2) + pow(y - o.y, 2));
		}

		Vector2 vscale(const Vector2& s) {
			return Vector2(x * s.x, y * s.y);
		}

		Vector2 scale(float s) {
			return Vector2(x * s, y * s);
		}

		Vector2 normalize() {
			float l = length();
			return Vector2(x / l, y / l);
		}

		Vector2 add(const Vector2& o) {
			return Vector2(x + o.x, y + o.y);
		}

		Vector2 sub(const Vector2& o) {
			return Vector2(o.x - x, o.y - y);
		}

		Vector2 clone() {
			return Vector2(x, y);
		}

		float polar() const
		{
			if (abs(x - 0) <= 1e-9)
			{
				if (y > 0.f)
				{
					return 90.f;
				}
				return y < 0.f ? 270.f : 0.f;
			}

			auto theta = atan(y / x) * (180.0 / 3.1415936f);

			if (x < 0.f)
			{
				theta = theta + 180.;
			}
			if (theta < 0.)
			{
				theta = theta + 360.;
			}
			return (float)theta;
		}

		float angle_between(const Vector2& other) const
		{
			auto theta = polar() - other.polar();
			if (theta < 0.f)
			{
				theta = theta + 360.f;
			}
			if (theta > 180.f)
			{
				theta = 360.f - theta;
			}
			return theta;
		}

		Vector2 extend(Vector2& to, float range)
		{
			auto substracted = sub(to);
			auto norm = substracted.normalize();
			auto sacled = norm.scale(range);

			return add(sacled);
		}
	};

	struct Vector3 {
		Vector3() {};
		Vector3(float _x, float _y, float _z) {
			x = _x;
			y = _y;
			z = _z;
		}

		float x = 0.0;
		float y = 0.0;
		float z = 0.0;


		void reset()
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
		bool isValid() {
			//return x > 0.0f && y != 0.0f && z > 0.0f && x < 15000.0f && y < 15000.0f && z < 15000.0f;
			return x != 0.0 && y != 0.0f && x > -15000.0f && y != 0.0f && z > -15000.0f && x < 15000.0f && y < 15000.0f && z < 15000.0f;
		}

		float length() const {
			return (float)sqrt(x * x + y * y + z * z);
		}

		float distance(const Vector3& o) const {
			return sqrtf(powf(x - o.x, 2) + powf(y - o.y, 2) + powf(z - o.z, 2));
		}

		Vector3 rotate_x(float angle) {
			return Vector3(
				x,
				y * (float)cos(angle) - z * (float)sin(angle),
				y * (float)sin(angle) + z * (float)cos(angle)
			);
		}

		Vector3 rotate_y(float angle) {
			return Vector3(
				x * (float)cos(angle) + z * (float)sin(angle),
				y,
				-x * (float)sin(angle) + z * (float)cos(angle)
			);
		}

		Vector3 rotate_z(float angle) {
			return Vector3(
				x * (float)cos(angle) - y * (float)sin(angle),
				x * (float)sin(angle) + y * (float)cos(angle),
				z
			);
		}

		float dot(const Vector3& p2) {
			return x * p2.x + y * p2.y + z * p2.z;
		}

		Vector3 cross(const Vector3& p2) const {

			return Vector3(y * p2.z - z * p2.y, z * p2.x - x * p2.z, x * p2.y - y * p2.x);
		}


		Vector3 invert() {

			return Vector3(-x,-y,-z);
		}

		Vector3 vscale(const Vector3& s) {
			return Vector3(x * s.x, y * s.y, z * s.z);
		}

		Vector3 direction(const Vector3& from) const {
			return from.sub(*this);
		}

		Vector3 scale(float s) {
			return Vector3(x * s, y * s, z * s);
		}

		Vector3 perpendicular(bool right_direction = false) {
			if (right_direction)
				return Vector3(z, y, -x);

			return Vector3(-z, y, x);
		}

		Vector3 normalize() {
			float len = length();
			return Vector3(x / len, y / len, z / len);
		}

		Vector3 add(const Vector3& o) const {
			return Vector3(x + o.x, y + o.y, z + o.z);
		}

		Vector3 sub(const Vector3& o) const {
			return Vector3(o.x - x, o.y - y, o.z - z);
		}

		Vector3 clone() {
			return Vector3(x, y, z);
		}


		Vector3 operator/=(float divisor) {
			
			return	{
				x /= divisor,
				y /= divisor,
				z /= divisor};
		}

		Vector3 operator/(float divisor) {
			return	{
			x /= divisor,
			y /= divisor,
			z /= divisor };
		}

		Vector3 extend(const Vector3& to, float range)
		{
			auto substracted = sub(to);
			auto norm = substracted.normalize();
			auto sacled = norm.scale(range);

			return add(sacled);
		}

		Vector3 Rotated(float angle) const
		{
			auto const c = cos(angle);
			auto const s = sin(angle);

			return { static_cast<float>(x * c - z * s), y, static_cast<float>(z * c + x * s) };
		}

		Vector3 ProjectionOn(const Vector3& vOther) const
		{
			Vector3 toProject = Vector3(x, y, z);
			Vector3 vOther1 = vOther;
			float scale = (toProject.dot(vOther)) / (vOther1.dot(vOther));
			return vOther1 * scale;
		}

		float angel_between(const Vector3& other) const {
			return acos((this->x * other.x + this->y * other.y + this->z * other.z) / (this->length() * other.length())) * (180.0f / 3.141592654f);
		}

		float magnitude(const Vector3& v) {
			return std::sqrt(this->dot(v));
		}

		/*
		float angle_between(const Vector3& v, const Vector3& w) {
			Vector3 this_v = this->cross(v);
			Vector3 vw = v.cross(w);
			return std::atan2(magnitude(this_v), this_v.dot(vw)) * 180 / 3.1415926536;
		}*/


		float angleBetween2(Vector3 v)
		{
			this->normalize();
			v.normalize();
			return std::acos(this->dot(v));
		}

		bool operator ==(const Vector3& o) const {
			return (x == o.x && y == o.y && z == o.z);
		}

		Vector3 operator+(const Vector3& other) const
		{
			return { x + other.x, y + other.y, z + other.z };
		}

		Vector3 operator+(float scalar) const
		{
			return { x + scalar, y + scalar, z + scalar };
		}

		Vector3 operator-(const Vector3& other) const
		{
			return { x - other.x, y - other.y, z - other.z };
		}

		Vector3 operator*(const Vector3& other) const
		{
			return { x * other.x, y * other.y, z * other.z };
		}

		Vector3 operator*(float scalar) const
		{
			return { x * scalar, y * scalar, z * scalar };
		}

		Vector3 operator*=(float scalar) const
		{
			return { x * scalar, y * scalar, z * scalar };
		}
	};

	struct Vector4 {
		Vector4() {};
		Vector4(float _x, float _y, float _z, float _w) {
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		}

		float x;
		float y;
		float z;
		float w;

		float length() {
			return (float)sqrt(x * x + y * y + z * z + w * w);
		}

		float distance(const Vector4& o) {
			return (float)sqrt(pow(x - o.x, 2) + pow(y - o.y, 2) + pow(z - o.z, 2) + pow(w - o.w, 2));
		}

		Vector4 vscale(const Vector4& s) {
			return Vector4(x * s.x, y * s.y, z * s.z, w * s.w);
		}

		Vector4 scale(float s) {
			return Vector4(x * s, y * s, z * s, w * s);
		}

		Vector4 normalize() {
			float l = length();
			return Vector4(x / l, y / l, z / l, w / l);
		}

		Vector4 add(const Vector4& o) {
			return Vector4(x + o.x, y + o.y, z + o.z, w + o.w);
		}

		Vector4 sub(const Vector4& o) {
			return Vector4(x - o.x, y - o.y, z - o.z, w - o.w);
		}

		Vector4 clone() {
			return Vector4(x, y, z, w);
		}
	};

	enum class ICON_IDS {
		aatrox_circle = 102,
		aatrox_square = 103,
		ahri_circle = 104,
		ahri_square = 105,
		akali_circle = 106,
		akali_square = 107,
		akshan_circle = 108,
		akshan_square = 109,
		alistar_circle = 110,
		alistar_square = 111,
		amumu_circle = 112,
		amumu_square = 113,
		anivia_circle = 114,
		anivia_square = 115,
		annie_circle = 116,
		annie_square = 117,
		aphelios_circle = 118,
		aphelios_square = 119,
		ashe_circle = 120,
		ashe_square = 121,
		aurelionsol_circle = 122,
		aurelionsol_square = 123,
		azir_circle = 124,
		azir_square = 125,
		bard_circle = 126,
		bard_square = 127,
		blitzcrank_circle = 128,
		blitzcrank_square = 129,
		brand_circle = 130,
		brand_square = 131,
		braum_circle = 132,
		braum_square = 133,
		caitlyn_circle = 134,
		caitlyn_square = 135,
		camille_circle = 136,
		camille_square = 137,
		cassiopeia_circle = 138,
		cassiopeia_square = 139,
		chogath_circle = 140,
		chogath_square = 141,
		corki_circle = 142,
		corki_square = 143,
		darius_circle = 144,
		darius_square = 145,
		diana_circle = 146,
		diana_square = 147,
		draven_circle = 148,
		draven_square = 149,
		drmundo_circle = 150,
		drmundo_square = 151,
		ekko_circle = 152,
		ekko_square = 153,
		elise_circle = 154,
		elise_square = 155,
		evelynn_circle = 156,
		evelynn_square = 157,
		ezreal_circle = 158,
		ezreal_square = 159,
		fiora_circle = 160,
		fiora_square = 161,
		fizz_circle = 162,
		fizz_square = 163,
		galio_circle = 164,
		galio_square = 165,
		gangplank_circle = 166,
		gangplank_square = 167,
		garen_circle = 168,
		garen_square = 169,
		gnar_circle = 170,
		gnar_square = 171,
		gragas_circle = 172,
		gragas_square = 173,
		graves_circle = 174,
		graves_square = 175,
		gwen_circle = 176,
		gwen_square = 177,
		hecarim_circle = 178,
		hecarim_square = 179,
		heimerdinger_circle = 180,
		heimerdinger_square = 181,
		illaoi_circle = 182,
		illaoi_square = 183,
		irelia_circle = 184,
		irelia_square = 185,
		ivern_circle = 186,
		ivern_square = 187,
		janna_circle = 188,
		janna_square = 189,
		jarvaniv_circle = 190,
		jarvaniv_square = 191,
		jax_circle = 192,
		jax_square = 193,
		jayce_circle = 194,
		jayce_square = 195,
		jhin_circle = 196,
		jhin_square = 197,
		jinx_circle = 198,
		jinx_square = 199,
		kaisa_circle = 200,
		kaisa_square = 201,
		kalista_circle = 202,
		kalista_square = 203,
		karma_circle = 204,
		karma_square = 205,
		karthus_circle = 206,
		karthus_square = 207,
		kassadin_circle = 208,
		kassadin_square = 209,
		katarina_circle = 210,
		katarina_square = 211,
		kayle_circle = 212,
		kayle_square = 213,
		kayn_circle = 214,
		kayn_square = 215,
		kennen_circle = 216,
		kennen_square = 217,
		khazix_circle = 218,
		khazix_square = 219,
		kindred_circle = 220,
		kindred_square = 221,
		kled_circle = 222,
		kled_square = 223,
		kogmaw_circle = 224,
		kogmaw_square = 225,
		leblanc_circle = 226,
		leblanc_square = 227,
		leesin_circle = 228,
		leesin_square = 229,
		leona_circle = 230,
		leona_square = 231,
		lillia_circle = 232,
		lillia_square = 233,
		lissandra_circle = 234,
		lissandra_square = 235,
		lucian_circle = 236,
		lucian_square = 237,
		lulu_circle = 238,
		lulu_square = 239,
		lux_circle = 240,
		lux_square = 241,
		malphite_circle = 242,
		malphite_square = 243,
		malzahar_circle = 244,
		malzahar_square = 245,
		maokai_circle = 246,
		maokai_square = 247,
		masteryi_circle = 248,
		masteryi_square = 249,
		missfortune_circle = 250,
		missfortune_square = 251,
		monkeyking_circle = 252,
		monkeyking_square = 253,
		mordekaiser_circle = 254,
		mordekaiser_square = 255,
		morgana_circle = 256,
		morgana_square = 257,
		nami_circle = 258,
		nami_square = 259,
		nasus_circle = 260,
		nasus_square = 261,
		nautilus_circle = 262,
		nautilus_square = 263,
		neeko_circle = 264,
		neeko_square = 265,
		nidalee_circle = 266,
		nidalee_square = 267,
		nocturne_circle = 268,
		nocturne_square = 269,
		nunu_circle = 270,
		nunu_square = 271,
		olaf_circle = 272,
		olaf_square = 273,
		orianna_circle = 274,
		orianna_square = 275,
		ornn_circle = 276,
		ornn_square = 277,
		pantheon_circle = 278,
		pantheon_square = 279,
		poppy_circle = 280,
		poppy_square = 281,
		pyke_circle = 282,
		pyke_square = 283,
		qiyana_circle = 284,
		qiyana_square = 285,
		quinn_circle = 286,
		quinn_square = 287,
		rakan_circle = 288,
		rakan_square = 289,
		rammus_circle = 290,
		rammus_square = 291,
		reksai_circle = 292,
		reksai_square = 293,
		rell_circle = 294,
		rell_square = 295,
		renata_circle = 296,
		renata_square = 297,
		renekton_circle = 298,
		renekton_square = 299,
		rengar_circle = 300,
		rengar_square = 301,
		riven_circle = 302,
		riven_square = 303,
		rumble_circle = 304,
		rumble_square = 305,
		ryze_circle = 306,
		ryze_square = 307,
		samira_circle = 308,
		samira_square = 309,
		sejuani_circle = 310,
		sejuani_square = 311,
		senna_circle = 312,
		senna_square = 313,
		seraphine_circle = 314,
		seraphine_square = 315,
		sett_circle = 316,
		sett_square = 317,
		shaco_circle = 318,
		shaco_square = 319,
		shen_circle = 320,
		shen_square = 321,
		shyvana_circle = 322,
		shyvana_square = 323,
		singed_circle = 324,
		singed_square = 325,
		sion_circle = 326,
		sion_square = 327,
		sivir_circle = 328,
		sivir_square = 329,
		skarner_circle = 330,
		skarner_square = 331,
		sona_circle = 332,
		sona_square = 333,
		soraka_circle = 334,
		soraka_square = 335,
		swain_circle = 336,
		swain_square = 337,
		sylas_circle = 338,
		sylas_square = 339,
		syndra_circle = 340,
		syndra_square = 341,
		tahmkench_circle = 342,
		tahmkench_square = 343,
		taliyah_circle = 344,
		taliyah_square = 345,
		talon_circle = 346,
		talon_square = 347,
		taric_circle = 348,
		taric_square = 349,
		teemo_circle = 350,
		teemo_square = 351,
		thresh_circle = 352,
		thresh_square = 353,
		tristana_circle = 354,
		tristana_square = 355,
		trundle_circle = 356,
		trundle_square = 357,
		tryndamere_circle = 358,
		tryndamere_square = 359,
		twistedfate_circle = 360,
		twistedfate_square = 361,
		twitch_circle = 362,
		twitch_square = 363,
		udyr_circle = 364,
		udyr_square = 365,
		urgot_circle = 366,
		urgot_square = 367,
		varus_circle = 368,
		varus_square = 369,
		vayne_circle = 370,
		vayne_square = 371,
		veigar_circle = 372,
		veigar_square = 373,
		velkoz_circle = 374,
		velkoz_square = 375,
		vex_circle = 376,
		vex_square = 377,
		vi_circle = 378,
		vi_square = 379,
		viego_circle = 380,
		viego_square = 381,
		viktor_circle = 382,
		viktor_square = 383,
		vladimir_circle = 384,
		vladimir_square = 385,
		volibear_circle = 386,
		volibear_square = 387,
		warwick_circle = 388,
		warwick_square = 389,
		xayah_circle = 390,
		xayah_square = 391,
		xerath_circle = 392,
		xerath_square = 393,
		xinzhao_circle = 394,
		xinzhao_square = 395,
		yasuo_circle = 396,
		yasuo_square = 397,
		yone_circle = 398,
		yone_square = 399,
		yorick_circle = 400,
		yorick_square = 401,
		yuumi_circle = 402,
		yuumi_square = 403,
		zac_circle = 404,
		zac_square = 405,
		zed_circle = 406,
		zed_square = 407,
		zeri_circle = 408,
		zeri_square = 409,
		ziggs_circle = 410,
		ziggs_square = 411,
		zilean_circle = 412,
		zilean_square = 413,
		zoe_circle = 414,
		zoe_square = 415,
		zyra_circle = 416,
		zyra_square = 417,
	};

	static std::map<std::string, int> summoner_spell_icons = {
		{"summonerboost", 510},
		{ "summonerexhaust",511 },
		{ "summonerflash",512 },
		{ "summonerhaste",513 },
		{ "summonerheal",514 },
		{ "summonersmite",515 },
		{ "summonerteleport",516 },
		{ "summonerbarrier",517 },
		{ "summonerdot",518 }
	};






	class BestPredictionAOE
	{
	private:
		bool valid;
	public:

		Vector3 castPosition;
		int hitChance;
		int maxHits;
		std::vector<Vector3> positions;

		BestPredictionAOE()
		{
			valid = false;
		};

		BestPredictionAOE(Vector3 castPosition, int hitChance, int maxHits, std::vector<Vector3>  positions)
		{
			this->castPosition = castPosition;
			this->hitChance = hitChance;
			this->maxHits = maxHits;
			this->positions = positions;
			valid = true;
		};

		bool IsValid()
		{
			return valid;
		};
	};

	enum class PredictionHitchance {
		none,
		low,
		medium,
		high
	
	};
	
	enum class skillshot_type
	{
		SkillshotCircle,
		SkillshotLine,
		SkillshotMissileLine,
		SkillshotCone,
		SkillshotMissileCone,
		SkillshotRing,
		SkillshotArc
	};

	enum class spell_targeting
	{
		self,
		target,
		area,
		area_aim,
		cone,
		self_aoe,
		target_or_location,
		location,
		direction,
		direction_aim,
		drag_direction,
		line_target_to_caster,
		area_clamped,
		location_clamped,
		terrain_location,
		terrain_type,
		wall_detection,
		UNKNOWN
	};

	enum class Champions
	{
		Aatrox = 266,
		Ahri = 103,
		Akali = 84,
		Alistar = 12,
		Amumu = 32,
		Anivia = 34,
		Annie = 1,
		Aphelios = 523,
		Ashe = 22,
		AurelionSol = 136,
		Azir = 268,
		Bard = 432,
		Blitzcrank = 53,
		Brand = 63,
		Braum = 201,
		Caitlyn = 51,
		Camille = 164,
		Cassiopeia = 69,
		Chogath = 31,
		Corki = 42,
		Darius = 122,
		Diana = 131,
		DrMundo = 36,
		Draven = 119,
		Ekko = 245,
		Elise = 60,
		Evelynn = 28,
		Ezreal = 81,
		FiddleSticks = 9,
		Fiora = 114,
		Fizz = 105,
		Galio = 3,
		Gangplank = 41,
		Garen = 86,
		Gnar = 150,
		Gragas = 79,
		Graves = 104,
		Hecarim = 120,
		Heimerdinger = 74,
		Illaoi = 420,
		Irelia = 39,
		Ivern = 427,
		Janna = 40,
		JarvanIV = 59,
		Jax = 24,
		Jayce = 126,
		Jhin = 202,
		Jinx = 222,
		Kaisa = 145,
		Kalista = 429,
		Karma = 43,
		Karthus = 30,
		Kassadin = 38,
		Katarina = 55,
		Kayle = 10,
		Kayn = 141,
		Kennen = 85,
		Khazix = 121,
		Kindred = 203,
		Kled = 240,
		KogMaw = 96,
		Leblanc = 7,
		LeeSin = 64,
		Leona = 89,
		Lillia = 876,
		Lissandra = 127,
		Lucian = 236,
		Lulu = 117,
		Lux = 99,
		Malphite = 54,
		Malzahar = 90,
		Maokai = 57,
		MasterYi = 11,
		MissFortune = 21,
		Mordekaiser = 82,
		Morgana = 25,
		Nami = 267,
		Nasus = 75,
		Nautilus = 111,
		Neeko = 518,
		Nidalee = 76,
		Nocturne = 56,
		Nunu = 20,
		Olaf = 2,
		Orianna = 61,
		Ornn = 516,
		Pantheon = 80,
		Poppy = 78,
		Pyke = 555,
		Qiyana = 246,
		Quinn = 133,
		Rakan = 497,
		Rammus = 33,
		RekSai = 421,
		Rell = 526,
		Renekton = 58,
		Rengar = 107,
		Riven = 92,
		Rumble = 68,
		Ryze = 13,
		Samira = 360,
		Sejuani = 113,
		Senna = 235,
		Seraphine = 147,
		Sett = 875,
		Shaco = 35,
		Shen = 98,
		Shyvana = 102,
		Singed = 27,
		Sion = 14,
		Sivir = 15,
		Skarner = 72,
		Sona = 37,
		Soraka = 16,
		Swain = 50,
		Sylas = 517,
		Syndra = 134,
		TahmKench = 223,
		Taliyah = 163,
		Talon = 91,
		Taric = 44,
		Teemo = 17,
		Thresh = 412,
		Tristana = 18,
		Trundle = 48,
		Tryndamere = 23,
		TwistedFate = 4,
		Twitch = 29,
		Udyr = 77,
		Urgot = 6,
		Varus = 110,
		Vayne = 67,
		Veigar = 45,
		Velkoz = 161,
		Vi = 254,
		Viego = 234,
		Viktor = 112,
		Vladimir = 8,
		Volibear = 106,
		Warwick = 19,
		MonkeyKing = 62,
		Xayah = 498,
		Xerath = 101,
		XinZhao = 5,
		Yasuo = 157,
		Yone = 777,
		Yorick = 83,
		Yuumi = 350,
		Zac = 154,
		Zed = 238,
		Ziggs = 115,
		Zilean = 26,
		Zoe = 142,
		Zyra = 143,
		Gwen = 887,
		Akshan = 166,
		Vex = 711,
		Zeri = 221,
		Renata = 888,
		Belveth = 200,
		Nilah = 895,
		KSante = 897,
		Unknown = 5000,
		TFTChampion,
		SG_Skarner,
		SG_VelKoz,
		SG_RekSai,
		SG_KogMaw,
		SG_KhaZix,
		SG_ChoGath,
	};

	enum class PluginTypes {
		CHAMPION,
		UTILS
	};




	struct Keycodes {
		std::string key;
		int value;
	};

	static std::vector<Keycodes> keyValue = {
			{"1", 0x31},
			{"2", 0x32},
			{"3", 0x33},
			{"4", 0x34},
			{"5", 0x35},
			{"6", 0x36},
			{"7", 0x37},
			{"8", 0x38},
			{"9", 0x39},
			{"0", 0x30},
			{"A", 0x41},
			{"B", 0x42},
			{"C", 0x43},
			{"D", 0x44},
			{"E", 0x45},
			{"F", 0x46},
			{"G", 0x47},
			{"H", 0x48},
			{"I", 0x49},
			{"J", 0x4A},
			{"K", 0x4B},
			{"L", 0x4C},
			{"M", 0x4D},
			{"N", 0x4E},
			{"O", 0x4F},
			{"P", 0x50},
			{"Q", 0x51},
			{"R", 0x52},
			{"S", 0x53},
			{"T", 0x54},
			{"U", 0x55},
			{"V", 0x56},
			{"W", 0x57},
			{"X", 0x58},
			{"Y", 0x59},
			{"Z", 0x5A},
			{"SHIFT", 0x10},
			{"CTRL", 0x11},
			{"SPACE", 0x20},
			{"F1", 0x70},
			{"F2", 0x71},
			{"F3", 0x72},
			{"F4", 0x73},
			{"F5", 0x74},
			{"F6", 0x75},
			{"F7", 0x76},
			{"F8", 0x77},
			{"F9", 0x78},
			{"F10", 0x79},
			{"F11", 0x7A},
			{"F12", 0x7B},
			{",", VK_OEM_COMMA},
			{".", VK_OEM_PERIOD},
			{"-", VK_OEM_MINUS},
			{"<", VK_OEM_102},
			{"OEM5(`)", VK_OEM_5},
			{"MOUSEB1", VK_XBUTTON1},
			{"MOUSEB2", VK_XBUTTON2},
			{"MOUSE-MID", VK_MBUTTON},
			{"LMOUSE", VK_LBUTTON},
			{"INSERT", VK_INSERT}
	};

	enum class MenuArea {
		CORE,
		COMBO,
		HERASS,
		LANECLEAR,
		LASTHIT,
		UTILITY,
		VISUALS
	};



	enum class Events {
		onLoad,
		onUpdate,
		onBeforeAttack,
		onAfterAttack,
		onCreateObject,
		onDeleteObject,
		onGainBuff,
		onLoseBuff,
		onSpellCast,
		onMissileCreate,
		onMissileRemoved,
		onGapCloser,
		onInterruptable,
		onDraw,
		onMenu,
		onRecall,
		onActiveSpellCast
	};

	enum class UnitType {
		UNKNOWN,
		CHAMPION,
		MONSTER,
		MEELE,
		CASTER,
		SIEGE,
		SUPERMINION,
		JUNGLE,
		TURRET,
		INHIBITOR,
		OBJECT,
		REDBLUE,
		CRAB,
		DRAGON_BARON_HERALD
	};

	struct RGBA_COLOR {
		int _R = 0;
		int _G = 0;
		int _B = 0;
		int _A = 0;
		RGBA_COLOR(int R, int G, int B, int A = 255) : _R(R), _G(G), _B(B), _A(A) {};
	};

	enum class BuffTypes : uint8_t {

		Stun = 5,
		Invisibility = 6,
		Silence = 7,
		Taunt = 8,

		Polymorph = 10,
		Slow = 11,
		Snare = 12,

		SpellImmunity = 16,
		PhysicalImmunity = 17,
		Invulnerability = 18,

		Fear = 22,
		Charm = 23,
		Poison = 24,
		Suppression = 25,
		Blind = 26,

		Flee = 29,
		Knockup = 30,
		Knockback = 31,
		Disarm = 32,

		Drowsy = 34,
		Asleep = 35,

		Unkillable = 38,
		NONE = 9999
	};

	inline BuffTypes operator|(BuffTypes a, BuffTypes b) {
		return static_cast<BuffTypes>(static_cast<int>(a) | static_cast<int>(b));
	}
	inline BuffTypes operator&(BuffTypes a, BuffTypes b) {
		return static_cast<BuffTypes>(static_cast<int>(a) & static_cast<int>(b));
	}

	enum class SkillAimType
	{
		none,
		direction_to_target,
		mouse_on_target,
		direction_to_ally,
		direction_to_mouse,
		direction_to_ground,
		activate,
	};

	enum class SkillshotType
	{
		line,
		circle,
		cone
	};

	enum class CollisionableObjects
	{
		minions,
		heroes,
		yasuo_wall,
		walls,
		allies
	};

	enum class TargetSelectorTypes
	{
		health_absolute = 0,
		health_percent,
		distance_to_champ,
		distance_to_mouse,
		priority
	};


	enum class SpellSlot {
		Q, W, E, R, SUM1, SUM2, ITEM_1, ITEM_2, ITEM_3, ITEM_4, ITEM_5, ITEM_6, ITEM_7, INVALID,

	};

	static  std::string slot_to_string(SpellSlot slot)  {

		switch (slot)
		{
		case sdk::SpellSlot::Q:
			return "Q";
		case sdk::SpellSlot::W:
			return "W";
		case sdk::SpellSlot::E:
			return "E";
		case sdk::SpellSlot::R:
			return "R";
		case sdk::SpellSlot::SUM1:
			return "SUM1";
		case sdk::SpellSlot::SUM2:
			return "SUM2";
		case sdk::SpellSlot::ITEM_1:
			return "ITEM_1";
		case sdk::SpellSlot::ITEM_2:
			return "ITEM_2";
		case sdk::SpellSlot::ITEM_3:
			return "ITEM_3";
		case sdk::SpellSlot::ITEM_4:
			return "ITEM_4";
		case sdk::SpellSlot::ITEM_5:
			return "ITEM_5";
		case sdk::SpellSlot::ITEM_6:
			return "ITEM_6";
		case sdk::SpellSlot::ITEM_7:
			return "ITEM_7";
		case sdk::SpellSlot::INVALID:
			return "INVALID";
		default:
			break;
		}

	};
	
	struct EvadeMissileInfo {

		std::string champion_name;
		int32_t heroNetworkId;
		bool isActiveSpell;
		sdk::SpellSlot slot;
		int networkId;
		std::string name;
		int32_t name_hash;
		Vector3 start;
		Vector3 end;
		std::function<Vector3()> position;
		float speed;
		float radius;
		float width;
		float delay;
		float range;
		float min_range;
		float max_range;
		skillshot_type type;
		spell_targeting targeting_type;
		float travel_time(Vector3 to){ 

			Vector3 pos;
			try {
				pos = position();
			}
			catch (const std::bad_function_call& e) {
				std::cerr << "[LOG] Error in travel_time(): " << e.what() << std::endl;
				return 0.0f;
			}

			if(pos.isValid())
				return  (pos.distance(to)) / speed;
			
			return 0.0f;
		};


		std::vector<Vector3> polygonPath;
		float starttime;
		bool to_delete = false;
		Vector3 first_intersection_point;
		Vector3 safe_position;
		Vector3 initial_unit_pos;


		bool isPassed() {

			Vector3 current_pos;
			try {
				current_pos = position();
			}
			catch (const std::bad_function_call& e) {
				std::cerr << "[LOG] Error in isPassed(): " << e.what() << std::endl;
				return true;
			}

			if (!current_pos.isValid() || !initial_unit_pos.isValid())
				return false;

			auto dist_start_unit = start.distance(initial_unit_pos);
			auto dist_start_pos = current_pos.distance(start);
			auto dist_pos_unit = current_pos.distance(initial_unit_pos);

			if (dist_pos_unit > width && dist_start_pos > dist_start_unit)
				return true;

			return false;
		};


	};

	struct UseQSS;

	struct QSS_Buff_Config {
		BuffTypes buff;
		bool* use = nullptr;
	};

	class BuffInstance
	{
	public:
		BYTE* orig_memory_address;
		BYTE* this_address;

		virtual BuffTypes BuffType() = 0;
		virtual std::string& Name() = 0;
		virtual float StartTime() = 0; //0x000C
		virtual float EndTime() = 0; //0x0010
		virtual float CoolDown() = 0; //0x0014
		virtual int  isAlive() = 0; //0x0024
		virtual int32_t OldStackCount() = 0; //0x0028
		virtual int32_t StackCount() = 0; //0x0074
		virtual int StackCount2() = 0;
		virtual int32_t MaxStackCount() = 0; //0x0078
		virtual int32_t SourceIndex() = 0; //0x0078
	};

	class BuffManagerInstance
	{
	public:
		virtual std::shared_ptr<BuffInstance> GetBuff(const std::string name, bool exact = true) = 0;
		virtual std::map<std::string, std::shared_ptr<BuffInstance>> GetAllBuffs() = 0;
		virtual bool hasBuffWithTypes(std::vector<BuffTypes> types = {}) = 0;
		virtual  void ReadBuffs() = 0;

	};

	enum TypeOfObject : int
	{
		//x << y = x*pow(2,y)
		//x >> y = x/pow(2,y)

		// GameObject = 1
		// AttackableObject = 1024


	/*	oGameObject = (1 << 0),  //0x1   (1024)
		oNeutralCamp = (1 << 1),  //0x2
		oDeadObject = (1 << 4),  //0x10
		oInvalidObject = (1 << 5),  //0x20
		oAIBaseCommon = (1 << 7),  //0x80
		oAttackableUnit = (1 << 9),  //0x200
		oAI = (1 << 10), //0x400
		oMinion = (1 << 11), //0x800
		oHero = (1 << 12), //0x1000
		oTurret = (1 << 13), //0x2000
		oUnknown0 = (1 << 14), //0x4000
		oMissile = (1 << 15), //0x8000
		oUnknown1 = (1 << 16), //0x10000
		oBuilding = (1 << 17), //0x20000
		oUnknown2 = (1 << 18), //0x40000*/

		oAttackableUnit = 2048,
		oHero = 4096,
		oTurret = 8192,
		oMissile= 32768,
		oBuilding = 131072
	
	};

	class TargetInstance;
	class ActiveSpellCastInstance;
	class SpellInstance;
	class ItemInstance;

	class GameObjectInstance {
	public:
		BYTE* orig_memory_address;
		BYTE* this_address;

		//dont use this in Plugins
		virtual void MemRead() = 0;
		//dont use this in Plugins
		virtual void LoadHeroIcons() = 0;
		//dont use this in Plugins
		virtual void* GetImage(const bool circled) = 0; //return ID3D11ShaderResourceView
		
		//returns the Champion name or on other Unit the name of that Unit
		virtual std::string ChampionName() = 0;
		//returns the hashed Champion name or on other Unit the name of that Unit
		virtual std::uint32_t ChampionNameHashed() = 0;

		//returns summoner name or on other units their names 
		virtual std::string ObjectName() = 0;

		//returns an ActiveSpellCastInstance pointer which then can be used to get infos of the current ActiveSpell
		virtual std::shared_ptr<ActiveSpellCastInstance>  GetActiveSpell() = 0;

		//returns an BuffManagerInstance pointer which then can be used to get infos of all buffs currently present on any champion
		virtual std::shared_ptr<BuffManagerInstance> GetBuffManager() =0;

		virtual int Team() = 0;
		virtual bool isAlly() = 0;
		//virtual bool isMoving() = 0;
		virtual bool aiIsMoving() = 0;
		virtual bool aiIsDashing() = 0;
		virtual float aiDashSpeed() = 0;
		virtual float aiMoveSpeed() = 0;
		virtual Vector3 aiClickPosition() = 0;
		virtual int aiCurrentPathSegment() = 0;
		virtual Vector3 aiPathStart() = 0;
		virtual Vector3 aiPathEnd() = 0;
		virtual Vector3 aiServerPosition() = 0;
		virtual std::vector<Vector3> aiFullPath() = 0;
		virtual bool UpdateClickPosition() = 0;
		virtual float aiGetLathPathChangeTime() = 0;
		virtual int GetHealthBarHeight() = 0;

		virtual Vector3 GetPerpendicularRightDirection() = 0;
		virtual Vector3 GetFacingDirection() = 0;
		virtual bool isFacing(const std::shared_ptr<GameObjectInstance>& unit, float max_angle = 90.0f) = 0;
		virtual bool isAlive() = 0;
		virtual bool isValid() = 0;
		virtual Vector3 Position() = 0;
		virtual UnitType SetType(UnitType u_type) = 0;
		virtual UnitType& GetType() = 0;

		/* returns if an object has a current, buff which is alive.
		* [buff] is a reference which can be used when hasBuff returns true, otherwise buff will be nullptr
		* [name] the name of the buff to be searched 
		* optional:
		* [exact] if this is true, it looks for the exact name, if it it false it will return true when any buff is found which has "name" as part of its string
		*/
		virtual bool hasBuff(std::shared_ptr < BuffInstance>& buff, std::string name, bool exact = true) = 0;

		virtual bool inAutoAttackRange(GameObjectInstance* to) = 0;
		virtual bool inRange(GameObjectInstance* to, float range) = 0;
		virtual float AttackRange() = 0;
		virtual float BaseAttackSpeedRatio() = 0;
		virtual float BaseAttackSpeed() = 0;
		virtual float AttackSpeed() = 0;
		virtual float BoundingRadius() = 0;
		virtual float BaseAttackCastTime() = 0;
		virtual float BaseAttackDelayCastOffsetPercent() = 0;
		virtual bool isAutoAttacking() = 0;

		/* returns true if the object is at least immobilized  by any of the given types
		* [types] an std::vector which has all BuffTypes this function should check for
	*/
		virtual bool immobilized(std::vector<BuffTypes> types = {}) = 0;
		virtual float BaseAttackRange() = 0;
		virtual int32_t Index() = 0; //0x0008
		virtual int32_t NetworkId() = 0; //0x00B4
		virtual bool isVisible() = 0; //0x0274
		virtual int8_t SpawnCount() = 0; //0x0288
		virtual float CurrentManaPercent() = 0; //0x029C
		virtual float CurrentMana() = 0; //0x029C
		virtual float MaxMana() = 0; //0x02AC
		virtual bool isInvulnearable() = 0; //0x03D4
		virtual bool isTargetable() = 0; //0x0D04
		virtual bool isRecalling() = 0; //0x0D90
		virtual float PredictedHealth(const float& missile_speed = 0.0f, const float& cast_delay = 0.0f) = 0;
		virtual float CurrentHealth() = 0; //0x0E74
		virtual float MaxHealth() = 0; //0x0E84
		virtual float BonusCDReduction() = 0; //0x119C
		virtual float TotalRawAttackDamage() = 0;
		virtual float TotalPhysicalDamage(GameObjectInstance * target, float rawdamage = -1) = 0;
		virtual float Lethality() =0;
		virtual float ArmorPen() =0; //0x12DC
		virtual float BonusAttackDamage() = 0; //0x12CC
		virtual float TotalAbilityPowerDamage(GameObjectInstance* target) = 0;
		virtual float TotalOverallDamage(GameObjectInstance* target, const float& ad_damage, const float& ap_damage) =0;
		virtual float AbilityPower() = 0; //0x12DC
		virtual float BonusAttackSpeed() = 0; //0x1324
		virtual float LifeSteal() = 0; //0x1338
		virtual float BonusOmnivamp() = 0; //0x1344
		virtual float AttackSpeedMulti() = 0; //0x1350
		virtual int GetCreepUpgrade() = 0;
		virtual float GetCreepAttackDamage() = 0;
		virtual float BaseDamage() = 0; //0x1354
		virtual float FullArmor() = 0; //0x137C
		virtual float BonusArmor() = 0; //0x1380
		virtual float MagicResist() = 0; //0x1384
		virtual float BonusMagicResist() = 0; //0x1388
		virtual float BaseHpRegPerSecond() = 0; //0x138C
		virtual float HpRegPerSecond() = 0; //0x1390
		virtual float MoveSpeed() = 0; //0x1394
		virtual float GetRawMovementSpeed(float extra_move_speed = 0.0) = 0; //0x1394
		virtual int32_t RespawnTimer() = 0; //0x1D00
		virtual int32_t RespawnTimer2() = 0; //0x1D10
		virtual float GetLastActiveSpellCastTime() = 0; //0x2CE8
		virtual Vector3 GroundPosition() = 0; //0x2F68
		virtual Vector3 MoveDirection() = 0; //0x348C
		virtual int32_t Level() = 0; //0x3584
		virtual int32_t SpellPointsAvailable() = 0; //0x35AC
		virtual bool isInEnemyTowerRange(GameObjectInstance*& out_tower, Vector3 adjusted_position = {}) = 0;
		virtual bool isInAllyTowerRange(GameObjectInstance*& out_tower, Vector3 adjusted_position = {}) = 0;
		virtual int32_t GetlastTargetedNetworkId() = 0;
		//returns the amount of allys to this unit in range or if  adjusted_position is given it returns ally in range of adjusted_position
		virtual int AllysInRange(float range, Vector3 adjusted_position = {}) =0;
		virtual bool inEnemyTurretRange() = 0;
		virtual bool inAllyTurretRange() = 0;

		//returns the amount of enemies to this unit. or if  adjusted_position is given it returns ally in range of adjusted_position
		virtual int EnemysInRange(float range, Vector3 adjusted_position = {}) =0;

		virtual bool isHero() = 0;
		virtual bool isMinion() = 0;
		virtual bool isNeutralCamp() = 0;
		virtual bool isDeadObject() = 0;
		virtual bool isTurret() = 0;
		virtual bool isBuilding() = 0;
		virtual bool isMissile() = 0;

		virtual bool is_caster() = 0;
		virtual bool is_meele() = 0;
		virtual bool is_cannon() = 0;
		virtual bool is_superminion() = 0;
		virtual bool is_minion() = 0;
		virtual bool is_tower() = 0;
		virtual bool is_nexus() = 0;
		virtual bool is_inhib() = 0;
		virtual bool is_red() = 0;
		virtual bool is_blue() = 0;
		virtual bool is_big_wolf() = 0;
		virtual bool is_mini_wolf() = 0;
		virtual bool is_gromp() = 0;
		virtual bool is_krug() = 0;
		virtual bool is_krug_mini() = 0;
		virtual bool is_krug_mini_mini() = 0;
		virtual bool is_yellow_trinket() = 0;
		virtual bool is_blue_trinket() = 0;
		virtual bool is_pink_ward() = 0;
		virtual bool is_razorbeak() = 0;
		virtual bool is_razorbeak_mini() = 0;
		virtual bool is_dragon() = 0;
		virtual bool is_baron() = 0;
		virtual bool is_herald() = 0;
		virtual bool is_crab() = 0;
		virtual bool is_jungle() = 0;

		virtual std::shared_ptr<SpellInstance> Q() = 0;
		virtual std::shared_ptr<SpellInstance> W() = 0;
		virtual std::shared_ptr<SpellInstance> E() = 0;
		virtual std::shared_ptr<SpellInstance> R() = 0;
		virtual std::shared_ptr<SpellInstance> SUM1() = 0;
		virtual std::shared_ptr<SpellInstance> SUM2() = 0;
		virtual std::shared_ptr<SpellInstance> ITEM1() = 0;
		virtual std::shared_ptr<SpellInstance> ITEM2() = 0;
		virtual std::shared_ptr<SpellInstance> ITEM3() = 0;
		virtual std::shared_ptr<SpellInstance> ITEM4() = 0;
		virtual std::shared_ptr<SpellInstance> ITEM5() = 0;
		virtual std::shared_ptr<SpellInstance> ITEM6() = 0;
		virtual std::shared_ptr<SpellInstance> ITEM7() = 0;

		//returns 0 if the item has not been found or a value from 1-7 for the item slots 1-7.
		virtual int  GetItemSlot(int item_id) = 0;

		//returns the ItemInstance of the item given at the specific slot 
		virtual std::shared_ptr<ItemInstance> GetItemBySlot(int slot) = 0;

		//returns true if this unit has the given itemid in his inventory
		virtual bool hasItem(int item_id) = 0;


		virtual float GetAverageClickSpeed() = 0;
		virtual float GetAverageClickAngle() = 0;

	
	};

	class TargetInstance : public virtual GameObjectInstance {
	public:
	};

	class SpellInstance {
	public:

		virtual uintptr_t*& This() = 0;
		virtual uintptr_t*& MemoryAddress() = 0;
		virtual void* GetIcon(const bool circled) = 0;

		/*
		* This registers a spell. If any parameter is set greater then 0.0f, this value will be used on the functions. if it is 0.0, the value will be read from memory.
		* Riot is inconsistent, so values from memory can be wrong:
		* Example: Ezreal Q range is 1100. Using SpellInstance::CastRange() will returns 500.0 from memory. So range should be set to 1100 in order to get the correct value from SpellInstance::CastRange()
		* _disable_windup_check : this mostly should not be set. It will disable the check if a myhero is currently casting any spell and if so it will wait until it finished that cast before casting another one.
		* In some situations you dont want to wait and so you can set _disable_windup_check = true.
		*/
		virtual void RegisterSpell(SkillAimType aim_type, float delay = 0.0, float range = 0.0, float radius = 0.0, float speed = 0.0, float line_width = 0.0, std::vector < CollisionableObjects> flags = {}, SkillshotType skillshot_type = SkillshotType::line, bool _disable_windup_check = false) = 0;
		
		//ignored
		virtual void Clone(SpellInstance& copy) = 0;
		virtual sdk::SkillAimType AimTtype() = 0;
		virtual sdk::SkillshotType SkillShotType() = 0;
		virtual sdk::SpellSlot Slot() = 0;
		virtual bool isReady() = 0;
		virtual bool isActivatable() = 0;
		virtual int Level() = 0;
		virtual int CaitCharges() = 0;
		virtual int Charges() = 0;
		virtual float Damage() = 0;
		virtual uint8_t StackCount() = 0;
		virtual float SpellReadyTime() = 0;
		virtual float& GetNextCastReadyTime() = 0;
		virtual float& GetLastTimeCasted() = 0;

		//virtual uintptr_t* mSpellSlotSpellInfo() = 0;
		virtual int32_t AffectsTypeFlags() = 0; //0x0010
		virtual std::string SpellName(char* name = nullptr) = 0; //0x006C
		virtual std::string AlternativeName() = 0; //0x006C
		virtual float* PhysicalDamage() = 0; //0x00D4 [6][4]
		virtual std::string AnimationName(char* name = nullptr) = 0; //0x0204
		virtual float CastDelay() = 0; //0x0264
		virtual bool isWindingUp() = 0;
		virtual float BaseCooldown() = 0; //0x0288
		virtual float DelayCastOffsetPercent() = 0; //0x02A4
		virtual float DelayTotalTimePercent() = 0; //0x02A8
		virtual float CastRange() = 0; //0x03D4
		virtual float CastRangeOverride() = 0; //0x03F0
		virtual float CastRadius() = 0; //0x040C
		virtual float CastFrame() = 0; //0x045C
		virtual float MissileSpeed() = 0; //0x0460
		virtual float LineWidth() = 0; //0x0494
		virtual int32_t FloatVarsDecimals() = 0; //0x04F8
		virtual float GetManaCosts() = 0; //0x052C
		virtual bool isCharging() = 0;
		virtual void Cast(std::shared_ptr<GameObjectInstance> to = nullptr, float charge_duration = 0.0, PredictionHitchance hitchance = PredictionHitchance::none) = 0;
		virtual void Cast(Vector3 pos, float charge_duration = 0.0f) = 0;
	};

	class LocalPlayerInstance : public virtual GameObjectInstance {
	public: 
		bool triggered_semi_cast_q = false;
		bool triggered_semi_cast_w = false;
		bool triggered_semi_cast_e = false;
		bool triggered_semi_cast_r = false;
		bool triggered_semi_cast_sum1 = false;
		bool triggered_semi_cast_sum2 = false;

		float m_last_autoattack = 0.0;
		bool m_attack_issued = false;
		std::shared_ptr<GameObjectInstance> last_target = nullptr;
		virtual float NaxtAutoAttack() = 0;

		virtual int GetWindup() = 0;

		//if myhero can autoattack (if not, its because AA is on cooldown)
		virtual bool CanAttack() = 0;

		//if myhero is charging a spell
		virtual bool isCharging() = 0;

		//Autoattacks the given target (moves the mouse to the target and attacks, moves the mouse back)
		virtual bool Attack(std::shared_ptr<GameObjectInstance> target) = 0;

		//returns if there was an autoattack before
		virtual bool& didAttack() = 0;

		//returns if myhero is in an AutoAttack animation right now
		virtual bool isAttacking() = 0;

		//returns if myhero is casting any spell right now
		virtual bool isCasting() = 0;

		//returns if myhero winding up for an autoattack (in this time, the AA can be canceld)
		virtual bool isWindingUp() = 0;

		//resets the AA timer, so it will instantly AA again if possible
		virtual void ResetAutoAttack() = 0;


		//returns if there was an autoattack right before
		virtual bool isAfterAttack() = 0;

		//returns if myhero is currently evading a spell
		virtual bool isEvading() = 0;

		//returns this basically disables evading when value is set to true. Need to be set back to false to enable it agin.
		virtual void ignoreEvade(bool value) = 0;

		//checks if evade is enabled or not
		virtual bool CanEvade() = 0;


		//returns the AutoAttack missile speed of you champion
		virtual float GetAutoAttackMissileSpeed() = 0;
	};

	class TargetSelectorInstance {
	public:

		std::unordered_map<long, std::shared_ptr<GameObjectInstance>>  m_all_turrets;
		std::unordered_map<long, std::shared_ptr<GameObjectInstance>>  m_all_inhibitors;
		std::unordered_map<long, std::shared_ptr<GameObjectInstance>> m_all_units;
		std::vector<std::shared_ptr<GameObjectInstance>> m_all_heros;


		// Returns a Target by given TargetSelectorTypes in the given range of this unit (normally myhero)
		virtual std::shared_ptr<GameObjectInstance> GetTarget(TargetSelectorTypes selection_type, float range, bool isAutoAttack = false) = 0;

		// Returns a LaneClear Target (any monster, no heros) in the given range of this unit. If AD or AP is given, it will return units whos health is below the given AD or AP value
		virtual std::shared_ptr<GameObjectInstance>  GetLaneClearTarget(float range, float ad = 0.0f, float ap = 0.0f) = 0;

		// Returns a LastHit Target (which is killable by your AA damage) (any monster, no heros) in the given range of this unit. If AD or AP is given, it will return units whos health is below the given AD or AP value
		virtual std::shared_ptr<GameObjectInstance>  GetLastHitTarget(float range, float ad = 0.0f, float ap = 0.0f) = 0;

		//ignore
		virtual void Clone(std::shared_ptr<TargetSelectorInstance>& copy) = 0;

		//returns a given unit by the units index 
		virtual std::shared_ptr<GameObjectInstance> GetTargetByIndex(int index) = 0;

		virtual std::shared_ptr<GameObjectInstance> GetTargetByNetworkID(const int id) = 0;
	};

	struct skillshot_Info
	{
		SpellSlot slot;
		bool is_dodgable = false;
		std::string champion_name;
		uint32_t name_hash;
		float delay;
		float speed;
		float radius;
		float width;
		float range;
		float min_range;
		float max_range;
		skillshot_type skillshot_type;
		spell_targeting targeting_type;

	};
	
	enum class OrbwalkModes {
		NONE,
		COMBO,
		HERASS,
		LANECLEAR,
		LASTHIT,
		FLEE,
		CUSTOM1,
		CUSTOM2
	};

	class OrbwalkerInstance {
	public:
		int clickspeed = 60.0f;
		virtual  OrbwalkModes Mode() = 0;
		virtual  bool isMode(OrbwalkModes mode) = 0;
		virtual  bool CanMove() = 0;
		virtual  bool Move() = 0;
		virtual  void AddWindUp(int windup) = 0;

		//disables Orbwalking or enables it
		virtual  void DisableMovement(bool stop_orbwalker) = 0;

		//returns if orbwalker has been disabled
		virtual  bool isMovementStopped() = 0;

		//set the orbwalking point to any Vector3 position on the map. To remove the custom orbwalker point, position needs to be set to nullptr.
		virtual  void SetOrbwalkingPoint(Vector3* position, bool dontMoveBack = false) = 0;

		//registers any custom key, which can be used from inside the Plugins (example: another key on LeeSin Plugin which can be used as "InsecKey"
		virtual void RegisterCustomKey(std::string name, int &key) = 0;
	};

	class ActiveSpellCastInstance {
	public:

		virtual std::string GetSpellName() = 0; //0x0018
		virtual bool& isValid() = 0; //0x0060
		virtual int GetSlot() = 0; //0x00C
		virtual int GetSlot2() = 0; //0x00C
		virtual int GetAttackCount() = 0; //0x0060
		virtual int GetSrcIndex() = 0; //0x006C
		virtual std::shared_ptr<GameObjectInstance> GetSourceObject() = 0; //0x006C
		virtual std::shared_ptr<GameObjectInstance> GetDestinationObject() = 0; //0x006C
		virtual int GetMissileNetID() = 0; //0x0078
		virtual Vector3 GetStartPosition() = 0; //0x0084
		virtual Vector3 GetEndPosition() = 0; //0x0090
		virtual Vector3 GetDirection() = 0; //0x00B4
		virtual int GetDestinationIndex() = 0; //0x00C0
		virtual int isBasic() = 0; //0x00C4
		virtual int32_t GetAimCount() = 0; //0x00C8 //when mouse is over a target it counts the targets this spell will hit. Example: Tristana W .. jumping while the mouse is hovered over a target , it show the number or targets which will get hit. On Click Spells its always 1
		virtual float GetWindUp() = 0; //0x00CC
		virtual float GetAttackDelay() = 0; //0x00DC
		virtual float GetCastStartTime() = 0; //0x00DC
		virtual float GetCastEndTime() = 0; //0x00DC
		virtual bool isAutoAttack() = 0;

		// SpellInfoSpellData
		virtual int32_t GetSpellInfoSlot() = 0;
		virtual int32_t GetAffectsTypeFlags() = 0;
		virtual char* GetUnitType() = 0;
		virtual std::string GetMissileName() = 0;
		virtual std::string GetAlternativeName() = 0;
		virtual float   GetDamage() = 0;
		virtual float* GetPhysicalDamage() = 0;
		virtual char* GetAnimationName() = 0;
		virtual float   GetCastTime() = 0;
		virtual float* GetCooldownTimes() = 0;
		virtual float   GetDelayCastOffsetPercent() = 0;
		virtual float   GetDelayTotalTimePercent() = 0;
		virtual float   GetCastRange() = 0;
		virtual float   GetCastRangeOverride() = 0;
		virtual float   GetCastRadius() = 0;
		virtual float   GetCastConeDistance() = 0;
		virtual int32_t GetCastType() = 0;
		virtual float   GetCastFrame() = 0;
		virtual float   GetMissleSpeed() = 0;
		virtual float   GetLineWidth() = 0;
		virtual char* GetHitBoneName() = 0;
		virtual int32_t GetFloatVarsDecimals() = 0;
		virtual float   GetManaCosts() = 0;
	};

	class SpellManagerInstance {
	public:
	};

	class ItemInstance {
	public:
		BYTE* orig_memory_address;
		BYTE* this_address;

		virtual void  Cast(Vector3 position = {}) = 0;
		virtual int GetSlot() = 0;
		virtual int GetId() = 0;
		virtual std::string ItemName() = 0;
		virtual bool isReady() = 0;
		virtual int itemCharges() = 0;
	};



	struct _USTRING {
		std::string key = "";
		int keycode = 0x0;
		bool clicked = false;
		int VKEY() {
			if (keycode > 0x0)
				return keycode;

			for (auto& ukey : keyValue) {
				if (key == ukey.key) {
					return ukey.value;
				}
			}
		}

	
	};
	struct Hotkeys {
		_USTRING menu = { "F6",VK_F6 ,false };		// Insert
		_USTRING combo = { "SPACE",VK_SPACE ,false };		// Sapace
		_USTRING attack_key = { "F11",VK_F11 ,false };	// F11
		_USTRING attack_champions_only = { "OEM_5",VK_OEM_5 ,false };	// F11
		_USTRING stop_movement = { "S",0x53 ,false };	// F11
		_USTRING ping_wards = { "H",0x48 ,false };	// F11
		_USTRING herass = { "V",0x56 ,false };			// v
		_USTRING laneclear = { "T",0x54 ,false };		// T
		_USTRING fast_laneclear = { "LMB",0x1 ,false };		// T
		_USTRING lasthit = { "X",0x58,false };			// X
		_USTRING flee = { "A", 0x41,false };			// X
		_USTRING custom1 = { "-1",-1 ,false };			// X
		_USTRING custom2 = { "-1",-1 ,false };			// X
		_USTRING q = { "Q",0x51 ,false };			// Q
		_USTRING w = { "W",0x57 ,false };			// W
		_USTRING e = { "E", 0x45,false };			// E
		_USTRING r = { "R",0x52 ,false };			// R
		_USTRING sum1 = { "D",0x44 ,false };		// D
		_USTRING sum2 = { "F",0x46 ,false };		// F
		_USTRING alt = { "ALT",VK_LMENU ,false };		// left alt
		_USTRING dodge_dangerous = { "SPACE",VK_SPACE ,false };		// left alt
		_USTRING toggle_evade = { "K",0x4B ,false };		// left alt

		_USTRING item_1 = { "1", 0x31,false };
		_USTRING item_2 = { "2", 0x32,false };
		_USTRING item_3 = { "3",0x33 ,false };
		_USTRING item_4 = { "4" ,0x34,false };
		_USTRING item_5 = { "5", 0x35,false };
		_USTRING item_6 = { "6", 0x36,false };
		_USTRING item_7 = { "7",0x37 ,false };


	};

	struct UseCombo {
		bool useq = false;
		bool usew = false;
		bool usee = false;
		bool user = false;

		bool use_semi_q = false;
		bool use_semi_w = false;
		bool use_semi_e = false;
		bool use_semi_r = false;
	};

	struct UseHerass {
		bool useq = false;
		bool usew = false;
		bool usee = false;
		bool user = false;

		bool use_semi_q = false;
		bool use_semi_w = false;
		bool use_semi_e = false;
		bool use_semi_r = false;

		float min_mana_percent = 50.0f;
	};

	struct UseLaneClear {
		bool useq = false;
		bool usew = false;
		bool usee = false;
		bool user = false;

		bool use_semi_q = false;
		bool use_semi_w = false;
		bool use_semi_e = false;
		bool use_semi_r = false;

		float min_mana_percent = 50.0f;
	};

	struct UseLastHit {
		bool useq = false;
		bool usew = false;
		bool usee = false;
		bool user = false;

		bool use_semi_q = false;
		bool use_semi_w = false;
		bool use_semi_e = false;
		bool use_semi_r = false;

		float min_mana_percent = 50.0f;
	};

	struct UseSummoners {
		bool use_flash = false;
		bool use_heal = false;
		bool use_barrier = false;
		float heal_barrier_health = 0.0;

		bool use_exhaust = false;
		float exhaust_health = 0.0;

		bool use_ignite = false;
		float ignite_health = 0.0;

		bool use_smite = false;
		bool draw_smite_status = true;
	};

	struct UsePotions {
		bool enabled = true;
		bool use_healt_potion = true;
		bool use_biscuit_cookie = false;
		bool use_refillable_potion = false;
		bool use_corrupting_potion = false;
		float use_potion_percentage = 60.0f;
	};



	struct UseQSS {
		bool enabled = false;
		bool use_cleanse = false;
		bool use_item = false;
		float delay = 0.0;

		std::map< BuffTypes, std::pair<std::string, bool>> immobilizations = {
			{BuffTypes::Stun		,{"Stun", false }},
			{BuffTypes::Silence     ,{"Silence", false}},
			{BuffTypes::Taunt		,{"Taunt", false}},
			{BuffTypes::Polymorph	,{"Polymorph", false}},
			{BuffTypes::Slow		,{"Slow", false}},
			{BuffTypes::Snare		,{"Snare", false}},
			{BuffTypes::Fear		,{"Fear", false}},
			{BuffTypes::Charm		,{"Charm", false}},
			{BuffTypes::Poison		,{"Poison", false}},
			{BuffTypes::Suppression	,{"Suppression", false}},
			{BuffTypes::Blind		,{"Blind", false}},
			{BuffTypes::Flee		,{"Flee", false}},
			{BuffTypes::Knockup		,{"Knockup", false}},
			{BuffTypes::Knockback	,{"Knockback", false}},
			{BuffTypes::Disarm		,{"Disarm", false}},
		//	{BuffTypes::Grounded	,{"Grounded", false}},
			{BuffTypes::Drowsy		,{"Drowsy", false}},
			{BuffTypes::Asleep		,{"Asleep", false}}
		};
	};


	class EvadeSettings {
		
	public:
		
		SpellSlot slot = SpellSlot::INVALID;
		bool enable = false;
		bool isDangerous = false;
		int dangerLevel = 2;
		bool draw = false;
			
		
	};

	class EvadeDodgeSpells {

	public:
		std::string champion_name;
		SpellSlot slot = SpellSlot::INVALID;
		bool enable = false;
		int dangerLevel = 2;
	};

	struct additional_item {
		std::string section;
		std::string key;
	};

	class SettingsInstance {
	public:
		std::map<std::string, bool> smitable_objects = {
													{ "All", false},
													{ "Baron",  true },
													{ "Herald",true },
													{ "Dragon",true },
													{ "Red",   false },
													{ "Blue",  false },
													{ "Crab",  false },
															};
		std::map<int, _USTRING> custom_semi_keys;

		std::map<std::string, additional_item> additional_items;
		std::string default_champion_plugin = "";
		std::string default_util_plugin = "";

		float menu_scale = 0.14f;
		bool recall_tracking = false;
		bool draw_hud = true;
		float scale_hud = 1.0;
		bool draw_camp_cleared = true;
		bool draw_spell_cooldowns = true;
		bool draw_lasthit_marker = true;
		bool draw_click_position = false;
		bool draw_aa_range_ally = false;
		bool draw_aa_range_enemy = false;
		bool draw_aa_range_myhero = true;
		bool draw_aa_range_towers = false;
		bool enable_ward_tracker = false;
		bool enable_evading = true;
		bool show_fps = true;
		bool always_evade = false;
		bool draw_evade_state = true;
		bool draw_last_seen = true;

		float recall_pos_x = 300.0f;
		float recall_pos_y = 800.0f;
		float recall_scale = 1.0f;

		bool gapcloser_to_all = true;

		float evade_delay = 0.0f;
		bool evade_only_dangerous = false;
		bool attack_selected_only = false;
		int input_delay = 20;
		int hold_zone = 0.0;
		bool draw_hold_zone = false;
		Hotkeys hotkey;
		UseCombo combo;
		UseHerass herass;
		UseLaneClear laneclear;
		UseLastHit lasthit;
		UseQSS QSS;
		UseSummoners summoners;
		UsePotions potions;
		TargetSelectorTypes targetselector;
		std::map<std::string, std::vector<EvadeSettings> > evade_settings;
		std::map<std::string, std::vector<EvadeDodgeSpells> > spells_to_dodge;
		
		//not implemented yet
		virtual void RegisterSemiSpell(_USTRING hotkey) = 0;
		virtual void SaveCore() = 0;
		virtual void SaveEvade() = 0;
		virtual void SaveChampion() = 0;
		
		virtual void LoadCore() = 0;
		virtual void LoadEvade() = 0;
		virtual void LoadChampion() = 0;
		
	
		/*
		*  this will load a value as Boolean from champions.ini
		*  [value] = the name of the value in the ini file
		*  
			optional:
		*  [section] = the section under which this value will be found. If this is not set or empty, it will take the current loaded champion name.
		*/
		virtual bool LoadCustomBool(std::string value, std::string section = "") = 0;

		/*
	*  this will load a value as Integer from champions.ini
	*  [value] = the name of the value in the ini file
	*
		optional:
	*  [section] = the section under which this value will be found. If this is not set or empty, it will take the current loaded champion name.
	*/
		virtual int LoadCustomInt(std::string value, std::string section = "") = 0;

		/*
	*  this will load a value as Float from champions.ini
	*  [value] = the name of the value in the ini file
	*
		optional:
	*  [section] = the section under which this value will be found. If this is not set or empty, it will take the current loaded champion name.
	*/
		virtual float LoadCustomFloat(std::string value, std::string section = "") = 0;

		/*
	*  this will load a value as std::string from champions.ini
	*  [value] = the name of the value in the ini file
	*
		optional:
	*  [section] = the section under which this value will be found. If this is not set or empty, it will take the current loaded champion name.
	*/
		virtual std::string LoadCustomString(std::string value, std::string section = "") = 0;
	};

	class MissileInstance {
	public:
		BYTE* orig_memory_address;
		BYTE* this_address;

		/**
		* The name this missile belongs to 
		*
		* @return  the name for the missile
		*/
		virtual std::string Name() = 0;

		/**
		* The team this missile belongs to (100, 200, 300).
		*
		* @return  the team number. 100 = order, 200 = order, 300 = jungle
		*/
		virtual int32_t Team() = 0; //0x0034

		/**
		* Networkid of the missile
		*
		* @return  the networkid of this missile
		*/
		virtual int32_t NetworkID() = 0; //0x00B4

		/**
		* current position of a missile. This method reads the current missile from memorx again using the memory address of that specific missile.
		* if the missile is not in memory anymore it can happen, that it still get read. In that cases the data returned is not valis anymore. Thats where the parameter networkid
		* can be used to check if ´the returned data is still a valid networkid.
		*
		* @return  Vector3 position of the missile.
		*/
		virtual Vector3 Position(int32_t networkid = 0) = 0; //0x01DC

		/**
		* Missiles are read only once from memory and the memory address is saved. This address get used when the current position of the missile is beeing read.
		* When a missile is not valid anymore, reading from memory will bring wrong values. Thats why this method is needed. It can be used, to check if the last networkid
		* is still the correct one.
		*
		* @return  true, if the networkid is the same as the one from the first time the missle spawned in memory
		*/
		virtual bool isValid(int32_t networkid) = 0;


		/**
		* The Vector3 start position the missile is casted from. Normally this is the object position
		*
		* @return the start position of this missile
		*/
		virtual Vector3 StartPos() = 0; //0x02E0

		/**
		* The Vector3 end position the missile is casted to. Some missles end position are automatically set to the max range (Ezreal Q). Others
		* use the mouse click position as end position. So make sure you tested it for the missile.
		*
		* @return the end position of this missile
		*/
		virtual Vector3 EndPos() = 0; //0x02EC

		/**
		* On targeted missiles, the target index is set. Its the current índex in memory of the target
		* 
		* @return the index of the target or 0 if not targeted
		*/
		virtual int32_t TargetIndex() = 0; //0x031C

		/**
		*  Missile is targeted (e.g. Vayne E or Auttoattacks, not targeted missiles are skillshots like Lee Sin Q)
		* 
		* @return returns 1 if the missile is targeted
		*/
		virtual int32_t isTargeted() = 0; //0x0320

		/**
		* The travel speed of the missile
		* 
		* @return the missile travel speed. This is inconsistent as its wrong for a lot of missiles, so it should be checked always.
		*/
		virtual float TravelSpeed() = 0; //0x0320

		virtual int32_t AffectsTypeFlags() = 0; //0x0010

		/**
		* unused
		*/
		virtual std::string UnitType() = 0; //0x0020

		/**
		* another name for the missile (not always set)
		* 
		* @return an other name for the missile as a string. This is inconsistent as its not set for a lot of missiles, so it should be checked always.
		*/
		virtual std::string AlternativeName() = 0; //0x0090

		/**
		* the animation name for the missile (not always set)
		* 
		* @return the animation name as a string. This is inconsistent as its not set for a lot of missiles, so it should be checked always.
		*/
		virtual std::string AnimationName() = 0; //0x0204

		/**
		* the time in milliseconds the missile needs before it spawns (kinda like the animation time) the most spells/missiles have 250 ms.
		* this value is more effective on ActiveSpellCast, as a missile is already spawned and so the casttime did already pass.
		*
		* @return the casttime in milliseconds.This is inconsistent as its not correct for a lot of missiles, so it should be checked always.
		*/
		virtual float CastTime() = 0; //0x0264

		/**
		* unused
		*
		*/
		virtual float DelayCastOffsetPercent() = 0; //0x02A4

		/**
		* unused
		*
		*/
		virtual float DelayTotalTimePercent() = 0; //0x02A8


		/**
		* radius of the missile (especially interesting for circular spells)
		*
		* @return the radius of the missile. This is inconsistent as its not correct for a lot of missiles, so it should be checked always.
		*/
		virtual float CastRadius() = 0;


		/**
		* width of the missile
		*
		* @return the width of the missile. This is inconsistent as its not correct for a lot of missiles, so it should be checked always.
		*/
		virtual float LineWidth() = 0; //0x0494


		/**
		* Slot (0,1,2,3 -> Q,W,E,R)
		*
		* @return the slot this missile belongs to
		*/
		virtual int Slot() = 0;


		/**
		* Timestamp of the cast start
		*
		* @return the gametime in milliseconds when this missle has been casted
		*/
		virtual float MissleTravelBeginTime() = 0;


		/**
		* range of the missile
		*
		* @return the max range of the missile. This is inconsistent as its not correct for a lot of missiles, so it should be checked always.
		*/
		virtual float MissileRange() = 0;

		/**
		* index of the casting object
		*
		* @return the current index in memory of the missile sender. 
		*/
		virtual int SourceIndex() = 0;


		/**
		*  CastType 0, 1, 2,3 . Kinda incosistent 
		*
		* @return the cast type of the missile but its inconsistent (3 is skillshot mostly)
		*/
		virtual int CastType() = 0;



		/**
		* The caster of the missile
		*
		* @return the GameObjectInstance or nullptr
		*/
		virtual std::shared_ptr<GameObjectInstance> GetSourceObject() = 0;


		/**
		* If the missile is targeted it returns the Object of the target
		*
		* @return If the missile is targeted it returns the Object of the target as GameObjectInstance otherwise its nullptr
		*/
		virtual std::shared_ptr<GameObjectInstance> GetDestinationObject() = 0; //0x006C
	};

	class MissileManagerInstance {
	public:
		uintptr_t* m_ptrMissleRoot = nullptr;
		std::map<int, std::shared_ptr<MissileInstance>> missile_map;
		std::vector<std::shared_ptr<MissileInstance>> m_all_missiles;

		virtual int ListSize() = 0;
		virtual void MemRead() = 0;
	};

	class GameRendererInstance {
	public:

	
		/**
		* Screen Width
		*
		* @return current screen width
		*/
		virtual uint32_t ScreenWidth() = 0;



		/**
		* Screen Height
		*
		* @return current screen height
		*/
		virtual int32_t ScreenHeight() = 0;
	

		/**
		* calculated the screen position of a given world position
		*
		* @param pos  the world position
		* @return Vector2 screen position
		*/
		virtual Vector2  WorldToScreen(const Vector3& pos) = 0;


		/**
		* calculates the minimap screen position of a given world position 
		*
		* @param pos  the world position
		* @return Vector2 screen minimap position
		*/
		virtual Vector2  WorldToMinimap(const Vector3& pos) const = 0;
	

		/**
		* Checks if a given Vector2 coordinate is on the screen or not
		*
		* @param point  the position
		* @param offsetX enlarges the x direction
		* @param offsetY enlarges the y direction
		*/
		virtual bool     IsOnScreen(const Vector2& point, float offsetX = 0.f, float offsetY = 0.f) const = 0;



		/**
		* Checks if a given Vector3 position is on the screen or not
		*
		* @param point  the position
		* @param offsetX enlarges the x direction 
		* @param offsetY enlarges the y direction
		*/
		virtual bool     IsWorldPointOnScreen(const Vector3& point, float offsetX = 0.f, float offsetY = 0.f) const = 0;
	};

	class DrawManagerInstance {
	public:
		std::map<int, bool> champions_to_gapclose;

		/**
		* This method is controlled by the core, dont call it as its starting another thread and will be buggy.
		*/
		virtual std::thread Run() = 0;


		/**
		* This method is controlled by the core, dont call.
		*/
		virtual HWND GetOverlayHandle() = 0;

		/**
		* The overlay is clickthrough by default whenever the menu is closed and its not clickthrough whenever the menu is open.
		* this method toggles the current state, but should not be used from plugins.
		*/
		virtual void ToggleClickThrough() = 0;

		/**
		* Draws a circle in 3d world space on the overlay
		*
		* @param worldPos  the center position of the circle
		* @param radius the radius of the circle
		* @param filled if the circle should be filled or transparent
		* @param numPoints the more points choosen, the more "circular" the drawing will be. More points = more performance needed. (Small circles only need 10-20 points)
		* @param optional color RGBA color of the rectangle
		* @param thickness border thickness of the rectangle

		*/
		virtual void DrawCircle3D(const Vector3& worldPos, float radius, bool filled, int numPoints, RGBA_COLOR color, float thickness = 2.0f) = 0;
		
		

		/**
		* Draws a circle in 2d world space on the overlay
		*
		* @param screen_pos  the center position of the circle which will be converted to screen position
		* @param radius the radius of the circle
		* @param filled if the circle should be filled or transparent
		* @param numPoints the more points choosen, the more "circular" the drawing will be. More points = more performance needed. (Small circles only need 10-20 points)
		* @param optional color RGBA color of the rectangle
		* @param thickness border thickness of the rectangle

		*/
		virtual void DrawCircle2D(const Vector3& screen_pos, float radius, bool filled, int numPoints, RGBA_COLOR color, float thickness) = 0;

		/**
		* Draws a circular image in 3d world space on the overlay. Only hero images do work for now. hero->GetImage()
		*
		* @param image  the image
		* @param pos the pos of the image on the world
		* @param radius the radius

		*/
		virtual void DrawCircularImage(void* image, const Vector3& pos, float radius) = 0;



		/**
		* Draws a rectangle in 3d world space on the overlay
		*
		* @param start  position start of the rectangle
		* @param end position end of the rectangle
		* @param w width of the rectangle
		* @param optional color RGBA color of the rectangle
		* @param optional filled to fill the rectangle
		*/
		virtual void DrawRect3D(Vector3 start, Vector3 end, float w, RGBA_COLOR color, float filled = false) = 0;
	
		

		/**
		* Draws a rectangle in 2d world space on the overlay
		*
		* @param top_left  top left position on screen 
		* @param bot_right bottom right position on screen 
		* @param filled if its a color filled rectangle
		* @param color RGBA color of the rectangle
		*/
		virtual void DrawRect2D(Vector2 top_left, Vector2 bot_right, bool filled, RGBA_COLOR color) = 0;
		/**
		* Draws a line on the overlay
		*
		* @param start  position start of the line
		* @param end position end of the line
		* @param w thickness of the line
		* @param optional color RGBA color of the line
		*/
		virtual void DrawLine(Vector3 start, Vector3 end, float w, RGBA_COLOR color) = 0;
		
		
		/**
		* Draws a polyline on the overlay
		*
		*
		* @param points the path of the polyline defined by Vector3 points
		* @param w thickness of the line
		* @param optional color RGBA color of the line
		*/
		virtual void DrawPolyLine(std::vector<Vector3> points, float w, RGBA_COLOR color) = 0;
	
		
		/**
		* Draws a Text on the overlay
		*
		*
		* @param text the text itself
		* @param size font size (e.g 15.0f)
		* @param position screen position of the text
		* @param color RGBA color of the text
		* @param optional offsetX moves the text in x direction
		* @param optional offsetY moves the text in y direction
		*/
		virtual void DrawText(std::string text, float size, Vector2 position, RGBA_COLOR color, float offsetX = 0.0f, float offsetY = 0.0f) = 0;


		/**
		* Starts drawing another ImGui Window on the overlay, if this method is used, EndWindow() needs to be called too. 
		* Everything between this methods will be drawn to that window.
		*
		* this function takes 6 parameters
		*
		* @param pos screen position
		* @param name name of the window
		* @param width width of the window in pixel
		* @param height height of the window in pixel
		* @param optional draggable to set the window draggable by mouse
		*/
		virtual void BeginWindow(Vector2 &pos, std::string name,  float width, float height, bool draggable = true) = 0;

		/**
		* Ends drawing of the ImGui Window.
		*/
		virtual void EndWindow() = 0;
	};



	class PredictionManagerInstance {
	public:


		/**
		* Calculates if a missile will collide with an object
		*
		* this function takes 6 parameters
		*
		* @param from start position of the missile
		* @param to end position of the missile
		* @param missile_width width of the missile.
		* @param target_obj the object which is beeing checked for a possible collision.
		* @param check_heros if enemy heros should be taken into account.
		* @param check_creeps if enemy creeps should be taken into account.
		* @param yasuo_wall if yasuo wall should be taken into account.
		* @return true when target_obj will get hit by the missile
		*/
		virtual bool WillCollide(const Vector3& from, const Vector3& to, float missile_width, GameObjectInstance* target_obj, bool check_heros, bool check_creeps,bool yasuo_wall = true) = 0;


		/**
		*  Checks if the given target_to_hit is on the missiles path and will therefore get hit by the missile unless target_to_hit does not dodge
		*
		* @param outHitPoint will receive the Vector3 position of the hit point if the missile will hit otherwise outHitPoint = Vector(0.0,0.0,0.0)
		* @param missile_from start position of the missile
		* @param missile_to  end position of the missile.
		* @param target_to_hit the object which is beeing checked if it will get hit.
		* @param missile_width width of the missile.
		* @return true when target_to_hit will get hit by the missile, otherwise false
		*/
		virtual bool isObjectOnMissilePath(Vector3& outHitPoint, Vector3 missile_from, Vector3 missile_to, GameObjectInstance* target_to_hit, float missile_width) = 0;

		/**
		*  Calculates the position a missile should be casted to hit the moving target. This should only be used with skillshot, not with spells which are directly spawning on the targets spot (like Karthus Q)
		*
		* @param missile_from start position of the missile
		* @param missile_to  end position of the missile.
		* @param projectileSpeed the speed of the missile
		* @param projectileRange the maximum range a missile is able to fly
		* @param delay the initial delay/animation time before a missile is beeing casted (most spells do have a 250ms delay).
		* @return a valid Vector3() position when a preedicted position has been found otherwise an invalid Vector(0.0,0.0,0.0).
		*/
		virtual Vector3 GetPredictedPositionSkillShot(GameObjectInstance* missile_from, GameObjectInstance* missile_to, const  float& projectileSpeed, const  float& projectileRange, const  float& delay) const = 0;


		/**
		*  Calculates the position the target will be after time_ms. This should be used for spells which are spawning directly like Karthus Q, Cassiopeia Q. 
		*
		* @param missile_from start position of the missile
		* @param missile_to  end position of the missile.
		* @param time_ms the time after which the spell spawns. (example: Cassiopeia Q spawns 400 milliseconds (time_ms = 0.4) + delay after casting).
		* @param delay the initial delay/animation time before a spell is beeing casted (most spells do have a 250ms delay). 
		* (Example: Cassiopeia Q has 250ms delay (delay = 0.25) so her Q spawns after total time  0.4+0.25 = 0.65s (650ms))
		* @return a valid Vector3() position when a preedicted position has been found otherwise an invalid Vector(0.0,0.0,0.0).
		*/
		virtual Vector3 GetPredictedPositionTimeBased(GameObjectInstance* missile_from, GameObjectInstance* missile_to, const float& time_ms, const float& delay) const = 0;

		/**
		*  Checks if a given position is a Wall
		*
		* @param position the position to check
		* @param a wall can be a wall for one team but "no wall" for the other team (example the gate on the base, where only allys can walk through). 2048 = own team , 4096 = enemy team (default = 2048).
		* @return true, when the given position is a wall, otherwise false
		*/
		virtual bool isWall(const Vector3& position, unsigned __int16 team = 2048) = 0;

		/**
		*  Checks if a given position is a bush
		*
		* @param position the position to check
		* @param team should be default 2048
		* @return true, when the given position is a bush, otherwise false
		*/
		virtual bool isBush(const Vector3& position, unsigned __int16 team = 2048) = 0;

		/**
		*  Checks if a given position is the ground
		*
		* @param position the position to check
		* @param team should be default 2048
		* @return true, when the given position is ground, otherwise false
		*/
		virtual bool isGround(const Vector3& position, unsigned __int16 team = 2048) = 0;

		/**
		*  Checks if a given position is a structure like inhibitor, tower, nexus
		*
		* @param position the position to check
		* @param team should be default 2048
		* @return true, when the given position is as structure, otherwise false
		*/
		virtual bool isStructure(const Vector3& position, unsigned __int16 team = 2048) = 0;


		/**
		*  Calculated the angle between direction (Start|end1) and direction (start|end2)
		*
		* @param start the start poisition to create the direction vectors from
		* @param end1 the first end position which is been used to create direction (Start|end1)
		* @param end2 the second end position which is been used to create direction (Start|end2)
		* @return the angle in degrees between direction (Start|end1) and direction (start|end2)  (0-360°)
		*/
		virtual bool isBetween(const Vector3& start, const Vector3& end1, const Vector3& end2, float maxAngle) = 0;


		/**
		*  Checks if a given Vector3 point is inside a rectangle
		*
		* @param start position start of the rectangle
		* @param end position end of the rectangle
		* @param width width of the rectangle
		* @param point checks wether this point is inside the rect
		* @return the angle in degrees between direction (Start|end1) and direction (start|end2)  (0-360°)
		*/

		virtual bool isPointInRectangle(const Vector3& start, const Vector3& end, const float& width, const Vector3& point) const = 0;
	};

	class GameInstance {
	public:

		virtual void Initialize() = 0;

		/**
		*  Current Game Time
		* @return the current game time in milliseconds
		*/
		virtual float GameTime() = 0;

		/**
		*  Current World Mouseposition 
		* @return the current mouse position in the world
		*/
		virtual Vector3 GetMousePosition() = 0;

	    /**
		*  Current Screen Mouseposition 
		* @return the current mouse screen position
		*/
		virtual Vector2 GetMousePosition2D() = 0;

		/**
		*  Checks if the game is running in foreground
		* @return true when the Gamew is currently in foreground otherwise false
		*/
		virtual bool isInForeground() = 0;

		/**
		*  checks if the chat is open
		* @return true when the chat is opten otherwise false
		*/
		virtual bool isChatOpen() = 0;

		/**
		*  Checks if the shop window is open
		* @return true when the shop window is open otherwise false
		*/
		virtual bool isShopOpen() = 0;

		/**
		*  can be used to press any key 
		* 
		*  @param keycode should be a virtual keycode from https://learn.microsoft.com/de-de/windows/win32/inputdev/virtual-key-codes
		*/
		virtual void PressKey(int keycode) = 0;
	};



	enum class MenuItemType {

		CheckBox,
		DropDown,
		SubMenu,
		Slider,
		Collapsible,
		Seperator,
		Window,
		Hotkey


	};

	class MenuItemInstance
	{

	public:
		std::vector<std::string> log;
		/**
		*  Adds a single Item to another MenuItemInstance
		*
		* @param item: the item to be added. Depending on the order items are added, they will be shown in the menu in the same order
		*/
		virtual void addItem(MenuItemInstance* item) = 0;

		/**
		*  Adds a multiple Items to another MenuItemInstance.
		*
		* @param items: the items to be added. Depending on the order items are added, they will be shown in the menu in the same order
		* example: submenu->addItems({checkbox1, checkbox2, slider1, checkbox2}) -> this will place the items from top to bottom in the submenu
		*/
		virtual void addItems(std::vector<MenuItemInstance*> items) = 0;


		/**
		*  Redners the menu item, dont use it in plugins, its used in the core only
		*/
		virtual void render(std::string parent_sub) = 0;

		/**
		*  Redners the menu item, dont use it in plugins, its used in the core only
		*/
		virtual void render() = 0;

		/**
		*  @return the amout of current items a submenu or collapsible has, otherwise 0
		*/
		virtual int size() = 0;

		/**
		*  @return if a submenu or collapsible is open or closed
		*/
		virtual bool& isOpen() = 0;


		/**
		*  @return	CheckBox,
					DropDown,
					SubMenu,
					Slider,
					Collapsible,
					Seperator,
					Window,
					Hotkey
		*/
		virtual MenuItemType& GetType() = 0;


		/**
		*  Adds a checkbox to the list of menu items which will be shown when the menu is open.
		*
		* @param label the text which the menuitem will have when drawn
		* @param value needs to be a pointer to a bool. this value will hold the current state of the checkbox. 
		* @param optional tooltip an std::string which will be shown when hovering the checkbox with the mouse
		* @param optional do_save_to_settings if this value should be save to the champions ini file
		* @param optional section_name only for utility plugins. This defines the section name this plugin will save all values to in the ini file. Should be the plugin name or something unique.
		*/
		virtual void AddCheckbox(const std::string& label, bool* value, std::string tooltip = {}, bool do_save_to_settings = false, std::string section_name="") = 0;

				/**
		*  Adds a slider to the list of menu items which will be shown when the menu is open.
		* 
		* @param name the text which the menuitem will have when drawn
		* @param value the reference to the variable which holds the current state of the slider
		* @param min the minimum value a slider should have
		* @param max the maximum value the slider should have
		* @param optional tooltip an std::string which will be shown when hovering the checkbox with the mouse
		* @param optional do_save_to_settings if this value should be save to the champions ini file
		* @param optional section_name only for utility plugins. This defines the section name this plugin will save all values to in the ini file. Should be the plugin name or something unique.
		*/
		virtual void AddSliderInt(std::string name, int& value, int min, int max, std::string tooltip = {}, bool do_save_to_settings = false, std::string section_name = "") = 0;
		
		/**
		*  Adds a slider to the list of menu items which will be shown when the menu is open.
		*
		* @param name the text which the menuitem will have when drawn
		* @param value the reference to the variable which holds the current state of the slider
		* @param min the minimum value a slider should have
		* @param max the maximum value the slider should have
		* @param optional tooltip an std::string which will be shown when hovering the checkbox with the mouse
		* @param optional do_save_to_settings if this value should be save to the champions ini file
		* @param optional section_name only for utility plugins. This defines the section name this plugin will save all values to in the ini file. Should be the plugin name or something unique.
		*/
		virtual void AddSliderFloat(std::string name, float& value, float min, float max, std::string tooltip = {}, bool do_save_to_settings = false, std::string section_name = "") = 0;

		/**
		*  Adds seperator item to the menu
		*
		* @param description the description this seperator should have
		*/
		virtual void AddSeperator(std::string name) = 0;

		/**
		*  Adds a DropDown to the list of menu items which will be shown when the menu is open.
		*
		* @param name the text which the menuitem will have when drawn
		* @param selected_idx the reference to the variable which holds the current selected item of the dropdown
		* @param options an  std::vector<std::string> of selectable items the dropdown should have (from top top bottom)
		* @param optional tooltip an std::string which will be shown when hovering the checkbox with the mouse
		* @param optional do_save_to_settings if this value should be save to the champions ini file
		* @param optional section_name only for utility plugins. This defines the section name this plugin will save all values to in the ini file. Should be the plugin name or something unique.
		*/
		virtual void AddDropDown(std::string name, int& selected_idx, const std::vector<std::string>& options, std::string tooltip = {}, bool do_save_to_settings = false, std::string section_name = "") = 0;


	    /**
		*  Adds a Hotkey button to the list of menu items which will be shown when the menu is open.
		*
		* @param lable the text which the menuitem will have when drawn
		* @param key the reference to the variable which holds the keycode (https://learn.microsoft.com/de-de/windows/win32/inputdev/virtual-key-codes) of the hotkey
		* @param optional enabled if false, this hotkey cant be pressed.
		* @param optional tooltip an std::string which will be shown when hovering the checkbox with the mouse
		* @param optional do_save_to_settings if this value should be save to the champions ini file
		* @param optional section_name only for utility plugins. This defines the section name this plugin will save all values to in the ini file. Should be the plugin name or something unique.
		*/
		virtual void AddHotkey(const std::string& lable, int& key, bool enabled = true, std::string tooltip = {}, bool do_save_to_settings = false, std::string section_name = "") = 0;
	
		/**
		*  Adds a simple text/lable to the menu
		*
		* @param text the text to be displayed
		*/
		virtual void AddText(std::string& text) = 0;


		/**
		*  Checks if the given item is a core item or plugin item.Should most likely not be used from plugin side.
		*
		* @return true when the item is from core, otherwise false;
		*/
		virtual bool isCore() = 0;

	    /**
		*  Gets the name of the curent 
		* @return the label/name of the item.
		*/
		virtual  std::string& GetName() = 0;

	};



	class MainMenuInstance
	{
	public:
		


	   /**
	   *  Creates a submenu, which can be populated with other MenuItems
	   *
	   * @param name the text which the menuitem will have when drawn
	   * @param optional enabled when this is false, the menu item cant be clicked
	   * @param optional tooltip an std::string which will be shown when hovering the checkbox with the mouse
	   * @param optional icon this can be used to add a champion icon to the submenu, see example below.
	   * @param optional menuIcon can be used to add a unicode icon/emoji , which will be added in front of the submenus name
	   * 
	   * Example Icons:
	   * 	
	   *	
	   *	auto icon = myhero->GetImage(false);
	   *	if(icon != nullptr)
	   *       MenuItemInstance	* submenu = menu->CreateSubmenu(name,true,{}, icon);
	   *	else
	   *	   MenuItemInstance	* submenu  = menu->CreateSubmenu(name);
	   * 
	   */
		virtual MenuItemInstance* CreateSubmenu(std::string name, bool enabled = true, std::string tooltip = {}, void* icon = nullptr, std::string menuIcon = "") = 0;

	   /**
	   *  Creates a collapsible, which can be populated with other MenuItems
	   *
	   * @param name the text which the menuitem will have when drawn
	   * @param optional tooltip an std::string which will be shown when hovering the checkbox with the mouse
	   * @param optional icon this can be used to add a champion icon to the submenu, see example below.
	   * 
	   * Example Icons:
	   * 	
	   *	
	   *	auto icon = myhero->GetImage(false);
	   *	if(icon != nullptr)
	   *       MenuItemInstance	* submenu = menu->CreateCollapsible(name,{}, icon);
	   *	else
	   *	   MenuItemInstance	* submenu  = menu->CreateSubmenu(name);
	   * 
	   */
		virtual MenuItemInstance* CreateCollapsible(std::string name, std::string tooltip = {}, void* icon = nullptr) = 0;
	
		
	   /**
	   *  Creates a head of a treenode, which can be populated with other MenuItems
	   *
	   * @param name the text which the menuitem will have when drawn
	   * @param optional icon this can be used to add a champion icon to the submenu, see example below.
	   * 
	   */
		virtual MenuItemInstance* CreateTreeNode(std::string name, std::string tooltip = {}) = 0;

		/**
		*  Creates another seperate Window, which can be used to draw stuff permanent
		*
		* @param name the text which the menuitem will have when drawn
		* @param width in pixel the window should be created
		* @param height  in pixel the window should be created
		*
		*/
		virtual MenuItemInstance* CreateSubWindow(std::string name, int width, int height) = 0;
		
		/**
		*  Adds a single Item to another MenuItemInstance
		*
		* @param item: the item to be added. Depending on the order items are added, they will be shown in the menu in the same order
		*/
		virtual void addItem(MenuItemInstance* item) = 0;

		/**
		*  Adds a multiple Items to another MenuItemInstance.
		*
		* @param items: the items to be added. Depending on the order items are added, they will be shown in the menu in the same order
		* example: submenu->addItems({checkbox1, checkbox2, slider1, checkbox2}) -> this will place the items from top to bottom in the submenu
		*/
		virtual void addItems(std::vector<MenuItemInstance*> items) = 0;
		virtual void AddSeperator(std::string name) = 0;

		/**
		*  Redners the menu item, dont use it in plugins, its used in the core only
		*/
		virtual void render(std::string parent_sub = "") = 0;

		/**
		*  should be ignored
		*/
		virtual void cleanup() = 0;

		/**
		*  checks is a given subemnu or collapsible is open
		*/
		virtual bool& isOpen() = 0;

	};




	class Event {
	protected:
		Events m_type;
		std::string m_name;
		bool m_handled = false;
	public:
		Event() = default;
		Event(Events type, const std::string& name = "") : m_type(type), m_name(name) {};
		const Events type() const { return m_type; };
		const std::string& getName() const { return m_name; };
		const bool isHandled() { return m_handled; };
	private:
	};

	class EventHandlerManagerInstance {
	public:
		bool m_read_objectList = false;
		std::map<Events, std::vector<void*>> m_observers;
		virtual void subscribe(Events type, void* funct) = 0;
		virtual void add_callback(Events event, void* callback) = 0;
		virtual void remove_callback(Events event, void* callback) = 0;

		virtual void Update() = 0;
		virtual void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset = nullptr) = 0;
		virtual void BeforeAttack(GameObjectInstance* target, bool* process) = 0;
		virtual void SpellCast(GameObjectInstance* target) = 0;
		virtual void MissileCreate(std::shared_ptr<MissileInstance> missile) = 0;
		virtual void MissileRemoved(std::shared_ptr<MissileInstance> missile) = 0;
		virtual void BuffGain(std::shared_ptr < BuffInstance> sender) = 0;
		virtual void BuffLose(std::shared_ptr < BuffInstance> sender) = 0;
		virtual void Draw() = 0;
		virtual void Menu(MenuArea area) = 0;
		virtual void GapCloser(GameObjectInstance* sender) = 0;

		virtual void CreateObject(std::shared_ptr<GameObjectInstance> obj) = 0;
		virtual void DeleteObject(std::shared_ptr<GameObjectInstance> obj) = 0;
		virtual void ActiveSpell(std::shared_ptr<GameObjectInstance> sender, std::shared_ptr<ActiveSpellCastInstance> active_spell) = 0;
	};

	extern std::shared_ptr<EventHandlerManagerInstance> event_handler_manager;

	template < Events event >
	struct event_handler
	{
		static void add_callback() { }
		static void remove_handler() { }
	};

	template < >
	struct event_handler<Events::onAfterAttack>
	{
		static void add_callback(void(*callback)(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset)) { event_handler_manager->add_callback(Events::onAfterAttack, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset)) { event_handler_manager->remove_callback(Events::onAfterAttack, (void*)callback); }
	};
	template < >
	struct event_handler<Events::onBeforeAttack>
	{
		static void add_callback(void(*callback)(GameObjectInstance* target, bool* process)) { event_handler_manager->add_callback(Events::onBeforeAttack, (void*)callback); }
		static void remove_handler(void(*callback)(GameObjectInstance* target, bool* process)) { event_handler_manager->remove_callback(Events::onBeforeAttack, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onCreateObject>
	{
		static void add_callback(void(*callback)(std::shared_ptr<GameObjectInstance> obj)) { event_handler_manager->add_callback(Events::onCreateObject, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr<GameObjectInstance> obj)) { event_handler_manager->remove_callback(Events::onCreateObject, (void*)callback); }
	};
	template < >
	struct event_handler<Events::onDeleteObject>
	{
		static void add_callback(void(*callback)(std::shared_ptr<GameObjectInstance> obj)) { event_handler_manager->add_callback(Events::onDeleteObject, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr<GameObjectInstance> obj)) { event_handler_manager->remove_callback(Events::onDeleteObject, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onUpdate>
	{
		static void add_callback(void(*callback)()) { event_handler_manager->add_callback(Events::onUpdate, (void*)callback); }
		static void remove_handler(void(*callback)()) { event_handler_manager->remove_callback(Events::onUpdate, (void*)callback); }
	};
	template < >
	struct event_handler<Events::onSpellCast>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(GameObjectInstance* sender)) { event_handler_manager->add_callback(Events::onSpellCast, (void*)callback); }
		static void remove_handler(void(*callback)(GameObjectInstance* sender)) { event_handler_manager->remove_callback(Events::onSpellCast, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onMissileCreate>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(std::shared_ptr<MissileInstance> missile)) { event_handler_manager->add_callback(Events::onMissileCreate, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr<MissileInstance> missile)) { event_handler_manager->remove_callback(Events::onMissileCreate, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onMissileRemoved>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(std::shared_ptr<MissileInstance> missile)) { event_handler_manager->add_callback(Events::onMissileRemoved, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr<MissileInstance> missile)) { event_handler_manager->remove_callback(Events::onMissileRemoved, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onActiveSpellCast>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(std::shared_ptr<GameObjectInstance> sender, std::shared_ptr<ActiveSpellCastInstance> missile)) { event_handler_manager->add_callback(Events::onActiveSpellCast, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr<GameObjectInstance> sender, std::shared_ptr<ActiveSpellCastInstance> missile)) { event_handler_manager->remove_callback(Events::onActiveSpellCast, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onGainBuff>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(std::shared_ptr < BuffInstance> buff)) { event_handler_manager->add_callback(Events::onGainBuff, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr < BuffInstance> buff)) { event_handler_manager->remove_callback(Events::onGainBuff, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onLoseBuff>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(std::shared_ptr < BuffInstance> buff)) { event_handler_manager->add_callback(Events::onLoseBuff, (void*)callback); }
		static void remove_handler(void(*callback)(std::shared_ptr < BuffInstance> buff)) { event_handler_manager->remove_callback(Events::onLoseBuff, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onGapCloser>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(GameObjectInstance* sender)) { event_handler_manager->add_callback(Events::onGapCloser, (void*)callback); }
		static void remove_handler(void(*callback)(GameObjectInstance* sender)) { event_handler_manager->remove_callback(Events::onGapCloser, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onDraw>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)()) { event_handler_manager->add_callback(Events::onDraw, (void*)callback); }
		static void remove_handler(void(*callback)()) { event_handler_manager->remove_callback(Events::onDraw, (void*)callback); }
	};

	template < >
	struct event_handler<Events::onMenu>
	{//spellslot spell_slot, game_object_script target, vector& pos, vector& pos2, bool is_charge, bool* process
		static void add_callback(void(*callback)(MenuArea area)) { event_handler_manager->add_callback(Events::onMenu, (void*)callback); }
		static void remove_handler(void(*callback)(MenuArea area)) { event_handler_manager->remove_callback(Events::onMenu, (void*)callback); }
	};


	/*
	Global, static objects
	*/
	class sdk_manager {
	public:
		std::shared_ptr<EventHandlerManagerInstance> event_handler = nullptr;
		std::shared_ptr<LocalPlayerInstance> myhero = nullptr;
		std::shared_ptr<TargetSelectorInstance>  targetselector = nullptr;
		std::shared_ptr<OrbwalkerInstance>  orbwalker = nullptr;
		std::shared_ptr<DrawManagerInstance>  drawmanager = nullptr;
		std::shared_ptr<SpellManagerInstance>  spellmanager = nullptr;
	
		std::shared_ptr<SettingsInstance>  settings = nullptr;
		std::shared_ptr<MissileManagerInstance>  missilemanager = nullptr;
		std::shared_ptr<GameRendererInstance>  gamerenderer = nullptr;
		std::shared_ptr<GameInstance>  game = nullptr;
		std::shared_ptr<PredictionManagerInstance>  predictionmanager = nullptr;
		std::shared_ptr<MainMenuInstance>  menu = nullptr;
	};

	extern std::shared_ptr<sdk_manager> sdkmanager;
	extern std::shared_ptr<LocalPlayerInstance> myhero;
	extern std::shared_ptr<TargetSelectorInstance>  targetselector;
	extern std::shared_ptr<OrbwalkerInstance>  orbwalker;
	extern std::shared_ptr<DrawManagerInstance>  drawmanager;
	extern std::shared_ptr<SpellManagerInstance>  spellmanager;

	extern std::shared_ptr<SettingsInstance>  settings;
	extern std::shared_ptr<MissileManagerInstance>  missilemanager;
	extern std::shared_ptr<GameRendererInstance>  gamerenderer;
	extern std::shared_ptr<GameInstance>  game;
	extern std::shared_ptr<PredictionManagerInstance>  predictionmanager;
	extern 		std::shared_ptr<MainMenuInstance>  menu;


}



//#define CHAMPION(x)   const char* _champion = x;
#define CHAMPIONS(...)   const std::vector<const char* > _champions = {##__VA_ARGS__};
#define PLUGIN_TYPE(x)  const PluginTypes _plugin_type = x;
#define PLUGIN_NAME(x)  const char* _plugin_name = x;


extern "C" {
	BUZZ_API_EXPORT void link_plugins(std::shared_ptr<sdk::sdk_manager>  a_sdk);
	BUZZ_API_EXPORT void onLoad();
	BUZZ_API_EXPORT void onUnload();

	//BUZZ_API_EXPORT extern const char* _champion;
	BUZZ_API_EXPORT extern const std::vector<const char* > _champions;
	BUZZ_API_EXPORT extern const sdk::PluginTypes _plugin_type;
	BUZZ_API_EXPORT extern const char* _plugin_name;


	
}