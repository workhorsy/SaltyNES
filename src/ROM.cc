/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/


#include "SaltyNES.h"
#include "sha256sum.h"

const int ROM::VERTICAL_MIRRORING;
const int ROM::HORIZONTAL_MIRRORING;
const int ROM::FOURSCREEN_MIRRORING;
const int ROM::SINGLESCREEN_MIRRORING;
const int ROM::SINGLESCREEN_MIRRORING2;
const int ROM::SINGLESCREEN_MIRRORING3;
const int ROM::SINGLESCREEN_MIRRORING4;

const array<MapperStatus, 255> ROM::_mapperStatus = {
	MapperStatus(0, true, "NROM"),
	MapperStatus(1, true, "Nintendo MMC1"),
	MapperStatus(2, true, "UxROM"),
	MapperStatus(3, true, "CNROM"),
	MapperStatus(4, true, "Nintendo MMC3"),
	MapperStatus(5, false, "Nintendo MMC5"), // 5 games
	MapperStatus(6, false, "FFE F4xxx"),     // 6 games
	MapperStatus(7, true, "AxROM"),
	MapperStatus(8, false, "FFE F3xxx"),     // 8 games
	MapperStatus(9, true, "Nintendo MMC2"),
	MapperStatus(10, false, "Nintendo MMC4"),  // 2 games
	MapperStatus(11, true, "Color Dreams"),
	MapperStatus(12, false, "FFE F6xxx"),
	MapperStatus(13, false, "CPROM"),
	MapperStatus(14, false, ""),
	MapperStatus(15, false, "iNES Mapper #015"), // 3 games
	MapperStatus(16, false, "Bandai"),
	MapperStatus(17, false, "FFE F8xxx"),
	MapperStatus(18, true, "Jaleco SS8806"),
	MapperStatus(19, false, "Namcot 106"),
	MapperStatus(20, false, "(Hardware) Famicom Disk System"),
	MapperStatus(21, false, "Konami VRC4a, VRC4c"), // 2 games
	MapperStatus(22, false, "Konami VRC2a"), // 1 games
	MapperStatus(23, false, "Konami VRC2b, VRC4e, VRC4f"), // 5 games
	MapperStatus(24, false, "Konami VRC6a"),
	MapperStatus(25, false, "Konami VRC4b, VRC4d"),
	MapperStatus(26, false, "Konami VRC6b"),
	MapperStatus(27, false, ""),
	MapperStatus(28, false, ""),
	MapperStatus(29, false, ""),
	MapperStatus(30, false, ""),
	MapperStatus(31, false, ""),
	MapperStatus(32, false, "Irem G-101"),  // 4 games
	MapperStatus(33, false, "Taito TC0190, TC0350"), // 6 games
	MapperStatus(34, false, "BxROM, NINA-001"), // 6 games
	MapperStatus(35, false, ""),
	MapperStatus(36, false, ""),
	MapperStatus(37, false, ""),
	MapperStatus(38, false, ""),
	MapperStatus(39, false, ""),
	MapperStatus(40, false, ""),
	MapperStatus(41, false, "Caltron 6-in-1"),
	MapperStatus(42, false, ""),
	MapperStatus(43, false, ""),
	MapperStatus(44, false, ""),
	MapperStatus(45, false, ""),
	MapperStatus(46, false, "Rumblestation 15-in-1"),
	MapperStatus(47, false, "Nintendo MMC3 Multicart (Super Spike V'Ball + Nintendo World Cup)"),
	MapperStatus(48, false, "iNES Mapper #048"), // 1 games
	MapperStatus(49, false, ""),
	MapperStatus(50, false, ""),
	MapperStatus(51, false, ""),
	MapperStatus(52, false, ""),
	MapperStatus(53, false, ""),
	MapperStatus(54, false, ""),
	MapperStatus(55, false, ""),
	MapperStatus(56, false, ""),
	MapperStatus(57, false, ""),
	MapperStatus(58, false, ""),
	MapperStatus(59, false, ""),
	MapperStatus(60, false, ""),
	MapperStatus(61, false, ""),
	MapperStatus(62, false, ""),
	MapperStatus(63, false, ""),
	MapperStatus(64, false, "Tengen RAMBO-1"), // 3 games
	MapperStatus(65, false, "Irem H-3001"),
	MapperStatus(66, false, "GxROM"), // 7 games
	MapperStatus(67, false, "Sunsoft 3"),
	MapperStatus(68, false, "Sunsoft 4"), // 3 games
	MapperStatus(69, false, "Sunsoft FME-7"),
	MapperStatus(70, false, "iNES Mapper #070"),
	MapperStatus(71, false, "Camerica"), // 12 games
	MapperStatus(72, false, "iNES Mapper #072"),
	MapperStatus(73, false, "Konami VRC3"),
	MapperStatus(74, false, ""),
	MapperStatus(75, false, "Konami VRC1"),
	MapperStatus(76, false, "iNES Mapper #076 (Digital Devil Monogatari - Megami Tensei)"),
	MapperStatus(77, false, "iNES Mapper #077 (Napoleon Senki)"),
	MapperStatus(78, false, "Irem 74HC161/32"),
	MapperStatus(79, false, "American Game Cartridges"), // 9 games
	MapperStatus(80, false, "iNES Mapper #080"),
	MapperStatus(81, false, ""),
	MapperStatus(82, false, "iNES Mapper #082"),
	MapperStatus(83, false, ""),
	MapperStatus(84, false, ""),
	MapperStatus(85, false, "Konami VRC7a, VRC7b"),
	MapperStatus(86, false, "iNES Mapper #086 (Moero!! Pro Yakyuu)"),
	MapperStatus(87, false, "iNES Mapper #087"),
	MapperStatus(88, false, "iNES Mapper #088"),
	MapperStatus(89, false, "iNES Mapper #087 (Mito Koumon)"),
	MapperStatus(90, false, ""),
	MapperStatus(91, false, ""),
	MapperStatus(92, false, "iNES Mapper #092"),
	MapperStatus(93, false, "iNES Mapper #093 (Fantasy Zone)"),
	MapperStatus(94, false, "iNES Mapper #094 (Senjou no Ookami)"),
	MapperStatus(95, false, "iNES Mapper #095 (Dragon Buster) [MMC3 Derived]"),
	MapperStatus(96, false, "(Hardware) Oeka Kids Tablet"),
	MapperStatus(97, false, "iNES Mapper #097 (Kaiketsu Yanchamaru)"),
	MapperStatus(98, false, ""),
	MapperStatus(99, false, ""),
	MapperStatus(100, false, ""),
	MapperStatus(101, false, ""),
	MapperStatus(102, false, ""),
	MapperStatus(103, false, ""),
	MapperStatus(104, false, ""),
	MapperStatus(105, false, "NES-EVENT [MMC1 Derived]"),
	MapperStatus(106, false, ""),
	MapperStatus(107, false, ""),
	MapperStatus(108, false, ""),
	MapperStatus(109, false, ""),
	MapperStatus(110, false, ""),
	MapperStatus(111, false, ""),
	MapperStatus(112, false, ""),
	MapperStatus(113, false, "iNES Mapper #113"),
	MapperStatus(114, false, ""),
	MapperStatus(115, false, "iNES Mapper #115 (Yuu Yuu Hakusho Final) [MMC3 Derived]"),
	MapperStatus(116, false, ""),
	MapperStatus(117, false, ""),
	MapperStatus(118, false, "iNES Mapper #118 [MMC3 Derived]"),
	MapperStatus(119, false, "TQROM"),
	MapperStatus(120, false, ""),
	MapperStatus(121, false, ""),
	MapperStatus(122, false, ""),
	MapperStatus(123, false, ""),
	MapperStatus(124, false, ""),
	MapperStatus(125, false, ""),
	MapperStatus(126, false, ""),
	MapperStatus(127, false, ""),
	MapperStatus(128, false, ""),
	MapperStatus(129, false, ""),
	MapperStatus(130, false, ""),
	MapperStatus(131, false, ""),
	MapperStatus(132, false, ""),
	MapperStatus(133, false, ""),
	MapperStatus(134, false, ""),
	MapperStatus(135, false, ""),
	MapperStatus(136, false, ""),
	MapperStatus(137, false, ""),
	MapperStatus(138, false, ""),
	MapperStatus(139, false, ""),
	MapperStatus(140, false, "iNES Mapper #140 (Bio Senshi Dan)"),
	MapperStatus(141, false, ""),
	MapperStatus(142, false, ""),
	MapperStatus(143, false, ""),
	MapperStatus(144, false, ""),
	MapperStatus(145, false, ""),
	MapperStatus(146, false, ""),
	MapperStatus(147, false, ""),
	MapperStatus(148, false, ""),
	MapperStatus(149, false, ""),
	MapperStatus(150, false, ""),
	MapperStatus(151, false, ""),
	MapperStatus(152, false, "iNES Mapper #152"),
	MapperStatus(153, false, ""),
	MapperStatus(154, false, "iNES Mapper #152 (Devil Man)"),
	MapperStatus(155, false, ""),
	MapperStatus(156, false, ""),
	MapperStatus(157, false, ""),
	MapperStatus(158, false, ""),
	MapperStatus(159, false, "Bandai (Alternate of #016)"),
	MapperStatus(160, false, ""),
	MapperStatus(161, false, ""),
	MapperStatus(162, false, ""),
	MapperStatus(163, false, ""),
	MapperStatus(164, false, ""),
	MapperStatus(165, false, ""),
	MapperStatus(166, false, ""),
	MapperStatus(167, false, ""),
	MapperStatus(168, false, ""),
	MapperStatus(169, false, ""),
	MapperStatus(170, false, ""),
	MapperStatus(171, false, ""),
	MapperStatus(172, false, ""),
	MapperStatus(173, false, ""),
	MapperStatus(174, false, ""),
	MapperStatus(175, false, ""),
	MapperStatus(176, false, ""),
	MapperStatus(177, false, ""),
	MapperStatus(178, false, ""),
	MapperStatus(179, false, ""),
	MapperStatus(180, false, "(Hardware) Crazy Climber Controller"),
	MapperStatus(181, false, ""),
	MapperStatus(182, false, "iNES Mapper #182"),
	MapperStatus(183, false, ""),
	MapperStatus(184, false, "iNES Mapper #184"),
	MapperStatus(185, false, "iNES Mapper #185"),
	MapperStatus(186, false, ""),
	MapperStatus(187, false, ""),
	MapperStatus(188, false, ""),
	MapperStatus(189, false, ""),
	MapperStatus(190, false, ""),
	MapperStatus(191, false, ""),
	MapperStatus(192, false, ""),
	MapperStatus(193, false, ""),
	MapperStatus(194, false, ""),
	MapperStatus(195, false, ""),
	MapperStatus(196, false, ""),
	MapperStatus(197, false, ""),
	MapperStatus(198, false, ""),
	MapperStatus(199, false, ""),
	MapperStatus(200, false, ""),
	MapperStatus(201, false, ""),
	MapperStatus(202, false, ""),
	MapperStatus(203, false, ""),
	MapperStatus(204, false, ""),
	MapperStatus(205, false, ""),
	MapperStatus(206, false, ""),
	MapperStatus(207, false, "iNES Mapper #185 (Fudou Myouou Den)"),
	MapperStatus(208, false, ""),
	MapperStatus(209, false, ""),
	MapperStatus(210, false, ""),
	MapperStatus(211, false, ""),
	MapperStatus(212, false, ""),
	MapperStatus(213, false, ""),
	MapperStatus(214, false, ""),
	MapperStatus(215, false, ""),
	MapperStatus(216, false, ""),
	MapperStatus(217, false, ""),
	MapperStatus(218, false, ""),
	MapperStatus(219, false, ""),
	MapperStatus(220, false, ""),
	MapperStatus(221, false, ""),
	MapperStatus(222, false, ""),
	MapperStatus(223, false, ""),
	MapperStatus(224, false, ""),
	MapperStatus(225, false, ""),
	MapperStatus(226, false, ""),
	MapperStatus(227, false, ""),
	MapperStatus(228, false, "Active Enterprises"),
	MapperStatus(229, false, ""),
	MapperStatus(230, false, ""),
	MapperStatus(231, false, ""),
	MapperStatus(232, false, "Camerica (Quattro series)"),
	MapperStatus(233, false, ""),
	MapperStatus(234, false, ""),
	MapperStatus(235, false, ""),
	MapperStatus(236, false, ""),
	MapperStatus(237, false, ""),
	MapperStatus(238, false, ""),
	MapperStatus(239, false, ""),
	MapperStatus(240, false, ""),
	MapperStatus(241, false, ""),
	MapperStatus(242, false, ""),
	MapperStatus(243, false, ""),
	MapperStatus(244, false, ""),
	MapperStatus(245, false, ""),
	MapperStatus(246, false, ""),
	MapperStatus(247, false, ""),
	MapperStatus(248, false, ""),
	MapperStatus(249, false, ""),
	MapperStatus(250, false, ""),
	MapperStatus(251, false, ""),
	MapperStatus(252, false, ""),
	MapperStatus(253, false, ""),
	MapperStatus(254, false, ""),
};

ROM::ROM() : enable_shared_from_this<ROM>() {

}

shared_ptr<ROM> ROM::Init(shared_ptr<NES> nes) {
	failedSaveFile = false;
	saveRamUpToDate = true;
	header.fill(0);
	//rom = nullptr;
	//vrom = nullptr;
	//saveRam = nullptr;
	//vromTile = nullptr;
	this->nes = nes;
	romCount = 0;
	vromCount = 0;
	mirroring = 0;
	batteryRam = false;
	trainer = false;
	fourScreen = false;
	mapperType = 0;
	//string fileName;
	enableSave = true;
	valid = false;
	return shared_from_this();
}

ROM::~ROM() {
	closeRom();
	nes = nullptr;
}

string ROM::sha256sum(uint8_t* data, size_t length) {
	// Get the sha256 hash of the data
	unsigned char hash[32] = {0};
	SHA256Context ctx;
	SHA256Init(&ctx);
	SHA256Update(&ctx, data, length);
	SHA256Final(&ctx, hash);

	// Convert the hash into a string of hexadecimal values
	char hex_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	stringstream ss;
	for(size_t i=0; i<sizeof(hash); ++i) {
		uint8_t byte = hash[i];
		uint8_t upper_byte = byte >> 4;
		uint8_t lower_byte = byte & 0x0F;
		ss << hex_map[upper_byte];
		ss << hex_map[lower_byte];
	}

	return ss.str();
}

string ROM::getmapperName() {
	stringstream ss;
	ss << "Unknown Mapper, " << mapperType;
	return ss.str();
}

void ROM::load_from_data(string file_name, vector<uint8_t>* data, array<uint16_t, 0x2000>* save_ram) {
	fileName = file_name;
	auto sdata = vector<uint16_t>(data->size());
	for(size_t i=0; i<sdata.size(); ++i) {
		sdata[i] = static_cast<uint16_t>(data->data()[i] & 255);
	}
	log_to_browser("log: rom::load_from_data");

	// Get sha256 of the rom
	_sha256 = sha256sum(data->data(), data->size());
	log_to_browser("log: rom::sha256sum");

	// Read header:
	for(int i = 0; i < header.size(); ++i) {
		header[i] = sdata[i];
	}

	// Check first four bytes:
	if(sdata[0] != 'N' ||
	sdata[1] != 'E' ||
	sdata[2] != 'S' ||
	sdata[3] != 0x1A) {
		//System.out.println("Header is incorrect.");
		valid = false;
		return;
	}

	// Read header:
	romCount = header[4];
	vromCount = header[5] * 2; // Get the number of 4kB banks, not 8kB
	mirroring = ((header[6] & 1) != 0 ? 1 : 0);
	batteryRam = (header[6] & 2) != 0;
	trainer = (header[6] & 4) != 0;
	fourScreen = (header[6] & 8) != 0;
	mapperType = (header[6] >> 4) | (header[7] & 0xF0);

	printf("prog_rom_pages: %lu\n", static_cast<long>(romCount));
	printf("char_rom_pages: %lu\n", static_cast<long>(vromCount));
	printf("mirroring: %d\n", mirroring);
	printf("is_sram_on: %d\n", batteryRam);
	printf("is_trainer_on: %d\n", trainer);
	printf("mapper: %lu\n", static_cast<long>(mapperType));
	printf("sha256: %s\n", _sha256.c_str());

	// Battery RAM?
	saveRam = save_ram;
	if(batteryRam) {
		loadBatteryRam();
	}

	// Check whether byte 8-15 are zero's:
	bool foundError = false;
	for(int i = 8; i < 16; ++i) {
		if(header[i] != 0) {
			foundError = true;
			break;
		}
	}
	if(foundError) {
		// Ignore byte 7.
		mapperType &= 0xF;
	}

	rom = vector<array<uint16_t, 16384>>(romCount);
	for(size_t i=0; i<romCount; ++i) {
		rom[i].fill(0);
	}

	vrom = vector<array<uint16_t, 4096>>(vromCount);
	for(size_t i=0; i<vromCount; ++i) {
		vrom[i].fill(0);
	}

	vromTile = vector<array<Tile, 256>>(vromCount);
	for(size_t i=0; i<vromTile.size(); ++i) {
		for(size_t j=0; j<vromTile[i].size(); ++j) {
			vromTile[i][j] = Tile();
		}
	}

	//try{

	// Load PRG-ROM banks:
	size_t offset = 16;
	for(size_t i = 0; i < romCount; ++i) {
		for(size_t j = 0; j < 16384; ++j) {
			if(offset + j >= data->size()) {
				break;
			}
			rom[i][j] = sdata[offset + j];
		}
		offset += 16384;
	}

	// Load CHR-ROM banks:
	for(size_t i = 0; i < vromCount; ++i) {
		for(size_t j = 0; j < 4096; ++j) {
			if(offset + j >= data->size()) {
				break;
			}
			vrom[i][j] = sdata[offset + j];
		}
		offset += 4096;
	}

	// Create VROM tiles:
	for(size_t i = 0; i < vromCount; ++i) {
		for(size_t j = 0; j < 256; ++j) {
			vromTile[i][j] = Tile();
		}
	}

	// Convert CHR-ROM banks to tiles:
	//System.out.println("Converting CHR-ROM image data..");
	//System.out.println("VROM bank count: "+vromCount);
	int tileIndex = 0;
	int leftOver = 0;
	for(size_t v = 0; v < vromCount; ++v) {
		for(size_t i = 0; i < 4096; ++i) {
			tileIndex = i >> 4;
			leftOver = i % 16;
			if(leftOver < 8) {
				vromTile[v][tileIndex].setScanline(leftOver, vrom[v][i], vrom[v][i + 8]);
			} else {
				vromTile[v][tileIndex].setScanline(leftOver - 8, vrom[v][i - 8], vrom[v][i]);
			}
		}
	}

	/*
	tileIndex = (address+i)>>4;
	leftOver = (address+i) % 16;
	if(leftOver<8) {
	ptTile[tileIndex].setScanline(leftOver,value[offset+i],ppuMem.load(address+8+i));
	}else{
	ptTile[tileIndex].setScanline(leftOver-8,ppuMem.load(address-8+i),value[offset+i]);
	}
	*/

	/*}catch(Exception e) {
	//System.out.println("Error reading ROM & VROM banks. Corrupt file?");
	valid = false;
	return;
	}*/

	valid = true;
}

bool ROM::isValid() {
	return valid;
}

int ROM::getRomBankCount() {
	return romCount;
}

// Returns number of 4kB VROM banks.
int ROM::getVromBankCount() {
	return vromCount;
}

array<uint16_t, 16> ROM::getHeader() {
	return header;
}

array<uint16_t, 16384>* ROM::getRomBank(int bank) {
	return &(rom[bank]);
}

array<uint16_t, 4096>* ROM::getVromBank(int bank) {
	return &(vrom[bank]);
}

array<Tile, 256>* ROM::getVromBankTiles(int bank) {
	return &(vromTile[bank]);
}

int ROM::getMirroringType() {

	if(fourScreen) {
		return FOURSCREEN_MIRRORING;
	}

	if(mirroring == 0) {
		return HORIZONTAL_MIRRORING;
	}

	// default:
	return VERTICAL_MIRRORING;

}

size_t ROM::getMapperType() {
	return mapperType;
}

string ROM::getMapperName() {
	if(mapperType < getmapperName().length()) {
		return ROM::_mapperStatus[mapperType].name;
	}

	stringstream ss;
	ss << "Unknown Mapper, " << mapperType;
	return ss.str();

}

bool ROM::hasBatteryRam() {
	return batteryRam;
}

bool ROM::hasTrainer() {
	return trainer;
}

string ROM::getFileName() {
	return fileName;
}

bool ROM::mapperSupported() {
	return ROM::_mapperStatus[mapperType].is_supported;
}

shared_ptr<MapperDefault> ROM::createMapper() {

	if(!mapperSupported()) {
		fprintf(stderr, "Unsupported mapper: %zu for the rom: %s. Exiting ...\n", mapperType, fileName.c_str());
		exit(1);
	}

	switch(mapperType) {
			case 0: return make_shared<MapperDefault>()->Init(this->nes);
			case 1: return make_shared<Mapper001>()->Init(this->nes);
			case 2: return make_shared<Mapper002>()->Init(this->nes);
			case 3: return make_shared<Mapper003>()->Init(this->nes);
			case 4: return make_shared<Mapper004>()->Init(this->nes);
			case 7: return make_shared<Mapper007>()->Init(this->nes);
			case 9: return make_shared<Mapper009>()->Init(this->nes);
/*
			case 10: return new Mapper010();
*/
			case 11: return make_shared<Mapper011>()->Init(this->nes);
/*
			case 15: return new Mapper015();
			*/
			case 18: return make_shared<Mapper018>()->Init(this->nes);
			/*
			case 21: return new Mapper021();
			case 22: return new Mapper022();
			case 23: return new Mapper023();
			case 32: return new Mapper032();
			case 33: return new Mapper033();
			case 34: return new Mapper034();
			case 48: return new Mapper048();
			case 64: return new Mapper064();
			case 66: return new Mapper066();
			case 68: return new Mapper068();
			case 71: return new Mapper071();
			case 72: return new Mapper072();
			case 75: return new Mapper075();
			case 78: return new Mapper078();
			case 79: return new Mapper079();
			case 87: return new Mapper087();
			case 94: return new Mapper094();
			case 105: return new Mapper105();
			case 140: return new Mapper140();
			case 182: return new Mapper182();
*/
			default: return nullptr;
		}

	return nullptr;

}

void ROM::setSaveState(bool enableSave) {
	//this->enableSave = enableSave;
	if(enableSave && !batteryRam) {
		loadBatteryRam();
	}
}

array<uint16_t, 0x2000>* ROM::getBatteryRam() {
	return saveRam;
}

void ROM::loadBatteryRam() {
	if(batteryRam) {
		try {
			saveRamUpToDate = true;

			if(saveRam == nullptr) {
				saveRam = new array<uint16_t, 0x2000>();
				return;
			}

			//System.out.println("Battery RAM loaded.");
			if(nes->getMemoryMapper() != nullptr) {
				nes->getMemoryMapper()->loadBatteryRam();
			}
		} catch(exception& e) {
			//System.out.println("Unable to get battery RAM from user.");
			failedSaveFile = true;
		}
	}
}

void ROM::writeBatteryRam(int address, uint16_t value) {
	if(!failedSaveFile && !batteryRam && enableSave) {
		loadBatteryRam();
	}

	//System.out.println("Trying to write to battery RAM. batteryRam="+batteryRam+" enableSave="+enableSave);
	if(batteryRam && enableSave && !failedSaveFile) {
		(*saveRam)[address - 0x6000] = value;
		saveRamUpToDate = false;
	}
}

void ROM::closeRom() {
	if(batteryRam && !saveRamUpToDate) {
		try {
			// Create a message that has the game sha256 and saveram.
			stringstream out;
			out << "save:" << _sha256 << " data:";
			out << Misc::from_vector_to_hex_string(saveRam);
			log_to_browser(out.str().c_str());

			saveRamUpToDate = true;
			//System.out.println("Battery RAM sent to user.");
		} catch (exception& e) {
			//System.out.println("Trouble sending battery RAM to user.");
			//e.printStackTrace();
		}
	}
}
