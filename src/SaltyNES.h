/*
Copyright (c) 2012-2017 Matthew Brennan Jones <matthew.brennan.jones@gmail.com>
Copyright (c) 2006-2011 Jamie Sanders
A NES emulator in WebAssembly. Based on vNES.
Licensed under GPLV3 or later
Hosted at: https://github.com/workhorsy/SaltyNES
*/

#ifndef _SALTY_NES_H_
#define _SALTY_NES_H_


#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <pthread.h>
#include <string>
#include <algorithm>
#include <memory>
#include <array>
#include <sys/time.h>

#include "Color.h"
#include "base64.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_joystick.h>

#ifdef WEB
	#include <emscripten.h>
	#include <emscripten/bind.h>
#endif

using namespace std;

// Forward declarations
class IPapuChannel;
class ByteBuffer;
class ChannelDM;
class ChannelNoise;
class ChannelSquare;
class ChannelTriangle;
class CPU;
class CpuInfo;
class FileLoader;
class InputHandler;
class Logger;
class Mapper001;
class Mapper002;
class Mapper003;
class Mapper004;
class Mapper007;
class Mapper009;
class Mapper011;
class Mapper018;
class MapperDefault;
class Memory;
class Misc;
class NameTable;
class NES;
class PaletteTable;
class PAPU;
class PPU;
class Raster;
class ROM;
class Tile;
class SaltyNES;

// Interfaces
class IPapuChannel {
public:
	virtual ~IPapuChannel() { }
	virtual void writeReg(int address, int value) = 0;
	virtual void setEnabled(bool value) = 0;
	virtual bool isEnabled() = 0;
	virtual void reset() = 0;
	virtual int getLengthStatus() = 0;
};

class Parameters {
public:
	static string p1_up;
	static string p1_down;
	static string p1_left;
	static string p1_right;
	static string p1_a;
	static string p1_b;
	static string p1_start;
	static string p1_select;

	static string p2_up;
	static string p2_down;
	static string p2_left;
	static string p2_right;
	static string p2_a;
	static string p2_b;
	static string p2_start;
	static string p2_select;
};

// Class Prototypes
class Globals {
public:
	static SDL_Window* g_window;
	static SDL_Renderer* g_renderer;
	static SDL_Texture* g_screen;
	static bool is_windows;

	//static shared_ptr<NES> nes;
	static double CPU_FREQ_NTSC;
	static double CPU_FREQ_PAL;
	static int preferredFrameRate;

	// Microseconds per frame:
	static const double MS_PER_FRAME;
	// What value to flush memory with on power-up:
	static uint16_t memoryFlushValue;

	static bool disableSprites;
	static bool palEmulation;
	static bool enableSound;

	static std::map<string, uint32_t> keycodes; //Java key codes
	static std::map<string, string> controls; //vNES controls codes

	static std::map<int, SDL_Joystick*> joysticks;
};

class ByteBuffer {
public:
	static const int BO_BIG_ENDIAN = 0;
	static const int BO_LITTLE_ENDIAN = 1;

	vector<uint16_t> buf;
	int byteOrder = BO_BIG_ENDIAN;
	size_t curPos;
	bool hasBeenErrors;
	bool expandable = true;
	size_t expandBy = 4096;

	ByteBuffer(size_t size, const int byteOrdering);
	ByteBuffer(vector<uint8_t>* content, const int byteOrdering);
	void setExpandable(bool exp);
	void setExpandBy(size_t expBy);
	void setByteOrder(int byteOrder);
	uint8_t* getBytes();
	size_t getSize();
	size_t getPos();
	void error();
	bool hasHadErrors();
	void clear();
	void fill(uint8_t value);
	bool fillRange(size_t start, size_t length, uint8_t value);
	void resize(size_t length);
	void resizeToCurrentPos();
	void expand();
	void expand(size_t byHowMuch);
	void goTo(size_t position);
	void move(size_t howFar);
	bool inRange(size_t pos);
	bool inRange(size_t pos, size_t length);
	bool putBoolean(bool b);
	bool putBoolean(bool b, size_t pos);
	bool putByte(uint16_t var);
	bool putByte(uint16_t var, size_t pos);
	bool putShort(uint16_t var);
	bool putShort(uint16_t var, size_t pos);
	bool putInt(int var);
	bool putInt(int var, size_t pos);
	bool putString(string var);
	bool putString(string var, size_t pos);
	bool putChar(char var);
	bool putChar(char var, size_t pos);
	bool putCharAscii(char var);
	bool putCharAscii(char var, size_t pos);
	bool putStringAscii(string var);
	bool putStringAscii(string var, size_t pos);
	bool putByteArray(vector<uint16_t>* arr);
	bool readByteArray(vector<uint16_t>* arr);
	bool putShortArray(vector<uint16_t>* arr);
	string toString();
	string toStringAscii();
	bool readBoolean();
	bool readBoolean(size_t pos);
	uint16_t readByte();
	uint16_t readByte(size_t pos);
	uint16_t readShort();
	uint16_t readShort(size_t pos);
	int readInt();
	int readInt(size_t pos);
	char readChar();
	char readChar(size_t pos);
	char readCharAscii();
	char readCharAscii(size_t pos);
	string readString(size_t length);
	string readString(size_t pos, size_t length);
	string readStringWithShortLength();
	string readStringWithShortLength(size_t pos);
	string readStringAscii(size_t length);
	string readStringAscii(size_t pos, size_t length);
	string readStringAsciiWithShortLength();
	string readStringAsciiWithShortLength(size_t pos);
	//vector<uint16_t>* expandShortArray(vector<uint16_t>* array, int size);
	//void crop();
	//static ByteBuffer* asciiEncode(ByteBuffer* buf);
	//static ByteBuffer* asciiDecode(ByteBuffer* buf);
	//static void saveToZipFile(File f, ByteBuffer* buf);
	//static ByteBuffer* readFromZipFile(File f);
};

class ChannelDM : public IPapuChannel {
public:
	static const int MODE_NORMAL = 0;
	static const int MODE_LOOP = 1;
	static const int MODE_IRQ = 2;

	shared_ptr<PAPU> papu;
	bool _isEnabled;
	bool hasSample;
	bool irqGenerated;
	int playMode;
	int dmaFrequency;
	int dmaCounter;
	int deltaCounter;
	int playStartAddress;
	int playAddress;
	int playLength;
	int playLengthCounter;
	int shiftCounter;
	int reg4012, reg4013;
	int status;
	int sample;
	int dacLsb;
	int data;

	explicit ChannelDM();
	explicit ChannelDM(shared_ptr<PAPU> papu);
	virtual ~ChannelDM();
	void clockDmc();
	void endOfSample();
	void nextSample();
	void writeReg(int address, int value);
	void setEnabled(bool value);
	bool isEnabled();
	int getLengthStatus();
	int getIrqStatus();
	void reset();
};


class ChannelNoise : public IPapuChannel {
public:
	shared_ptr<PAPU> papu;
	bool _isEnabled;
	bool envDecayDisable;
	bool envDecayLoopEnable;
	bool lengthCounterEnable;
	bool envReset;
	bool shiftNow;
	int lengthCounter;
	int progTimerCount;
	int progTimerMax;
	int envDecayRate;
	int envDecayCounter;
	int envVolume;
	int masterVolume;
	int shiftReg;
	int randomBit;
	int randomMode;
	int sampleValue;
	uint32_t accValue;
	uint32_t accCount;
	int tmp;

	explicit ChannelNoise();
	explicit ChannelNoise(shared_ptr<PAPU> papu);
	virtual ~ChannelNoise();
	void clockLengthCounter();
	void clockEnvDecay();
	void updateSampleValue();
	void writeReg(int address, int value);
	void setEnabled(bool value);
	bool isEnabled();
	int getLengthStatus();
	void reset();
};

class ChannelSquare : public IPapuChannel {
public:
	static const int dutyLookup[32];
	static const int impLookup[32];

	shared_ptr<PAPU> papu;
	bool sqr1;
	bool _isEnabled;
	bool lengthCounterEnable;
	bool sweepActive;
	bool envDecayDisable;
	bool envDecayLoopEnable;
	bool envReset;
	bool sweepCarry;
	bool updateSweepPeriod;
	int progTimerCount;
	int progTimerMax;
	int lengthCounter;
	int squareCounter;
	int sweepCounter;
	int sweepCounterMax;
	int sweepMode;
	int sweepShiftAmount;
	int envDecayRate;
	int envDecayCounter;
	int envVolume;
	int masterVolume;
	int dutyMode;
	int sweepResult;
	int sampleValue;
	int vol;

	ChannelSquare();
	ChannelSquare(shared_ptr<PAPU> papu, bool square1);
	virtual ~ChannelSquare();
	void clockLengthCounter();
	void clockEnvDecay();
	void clockSweep();
	void updateSampleValue();
	void writeReg(int address, int value);
	void setEnabled(bool value);
	bool isEnabled();
	int getLengthStatus();
	void reset();
};

class ChannelTriangle : public IPapuChannel {
public:
	shared_ptr<PAPU> papu;
	bool _isEnabled;
	bool sampleCondition;
	bool lengthCounterEnable;
	bool lcHalt;
	bool lcControl;
	int progTimerCount;
	int progTimerMax;
	int triangleCounter;
	int lengthCounter;
	int linearCounter;
	int lcLoadValue;
	int sampleValue;
	int tmp;

	explicit ChannelTriangle();
	explicit ChannelTriangle(shared_ptr<PAPU> papu);
	virtual ~ChannelTriangle();
	void clockLengthCounter();
	void clockLinearCounter();
	int getLengthStatus();
	//int readReg(int address);
	void writeReg(int address, int value);
	void clockProgrammableTimer(int nCycles);
	void clockTriangleGenerator();
	void setEnabled(bool value);
	bool isEnabled();
	void updateSampleCondition();
	void reset();
};

class CPU : public enable_shared_from_this<CPU> {
public:
	// IRQ Types:
	static const int IRQ_NORMAL = 0;
	static const int IRQ_NMI	= 1;
	static const int IRQ_RESET  = 2;

	// References to other parts of NES :
	shared_ptr<NES> nes;
	shared_ptr<MapperDefault> mmap;
	vector<uint16_t>* mem;

	// Registers:
	int REG_ACC;
	int REG_X;
	int REG_Y;
	int REG_STATUS;
	int REG_PC;

	// Status flags:
	int F_CARRY;
	int F_ZERO;
	int F_INTERRUPT;
	int F_DECIMAL;
	int F_NOTUSED;
	int F_BRK;
	int F_OVERFLOW;
	int F_SIGN;

	// Misc. variables
	int opinf;
	int opaddr;
	int addrMode;
	int addr;
	int palCnt;
	int cycleCount;
	int cycleAdd;
	int temp;
	int add;

	// CPU Registers:
	int REG_ACC_NEW;
	int REG_X_NEW;
	int REG_Y_NEW;
	int REG_STATUS_NEW;
	int REG_PC_NEW;
	int REG_SP;

	// Status flags:
	int F_CARRY_NEW;
	int F_ZERO_NEW;
	int F_INTERRUPT_NEW;
	int F_DECIMAL_NEW;
	int F_BRK_NEW;
	int F_NOTUSED_NEW;
	int F_OVERFLOW_NEW;
	int F_SIGN_NEW;

	// Interrupt notification:
	bool irqRequested;
	int irqType;

	// Misc vars:
	int cyclesToHalt;
	bool stopRunning;
	bool crash;

	explicit CPU();
	shared_ptr<CPU> Init(shared_ptr<NES> nes);
	~CPU();
	void init();
	void stateLoad(ByteBuffer* buf);
	void stateSave(ByteBuffer* buf);
	void reset();
	void start();
	void stop();
	void emulate_frame();
	bool emulate();
	int load(int addr);
	int load16bit(int addr);
	void write(int addr, uint16_t val);
	void requestIrq(int type);
	void push(int value);
	void stackWrap();
	uint16_t pull();
	bool pageCrossed(int addr1, int addr2);
	void haltCycles(int cycles);
	void doNonMaskableInterrupt(int status);
	void doResetInterrupt();
	void doIrq(int status);
	int getStatus();
	void setStatus(int st);
	void setCrashed(bool value);
	void setMapper(shared_ptr<MapperDefault> mapper);
};

class CpuInfo {
public:
	// Opdata array:
	static bool isOp;
	static array<int, 256> opdata;
	// Instruction names:
	static const array<string, 56> instname;
	// Address mode descriptions:
	static const array<string, 13> addrDesc;
	static const array<int, 256> cycTable;
	// Instruction types:
	// -------------------------------- //
	static const int INS_ADC;
	static const int INS_AND;
	static const int INS_ASL;
	static const int INS_BCC;
	static const int INS_BCS;
	static const int INS_BEQ;
	static const int INS_BIT;
	static const int INS_BMI;
	static const int INS_BNE;
	static const int INS_BPL;
	static const int INS_BRK;
	static const int INS_BVC;
	static const int INS_BVS;
	static const int INS_CLC;
	static const int INS_CLD;
	static const int INS_CLI;
	static const int INS_CLV;
	static const int INS_CMP;
	static const int INS_CPX;
	static const int INS_CPY;
	static const int INS_DEC;
	static const int INS_DEX;
	static const int INS_DEY;
	static const int INS_EOR;
	static const int INS_INC;
	static const int INS_INX;
	static const int INS_INY;
	static const int INS_JMP;
	static const int INS_JSR;
	static const int INS_LDA;
	static const int INS_LDX;
	static const int INS_LDY;
	static const int INS_LSR;
	static const int INS_NOP;
	static const int INS_ORA;
	static const int INS_PHA;
	static const int INS_PHP;
	static const int INS_PLA;
	static const int INS_PLP;
	static const int INS_ROL;
	static const int INS_ROR;
	static const int INS_RTI;
	static const int INS_RTS;
	static const int INS_SBC;
	static const int INS_SEC;
	static const int INS_SED;
	static const int INS_SEI;
	static const int INS_STA;
	static const int INS_STX;
	static const int INS_STY;
	static const int INS_TAX;
	static const int INS_TAY;
	static const int INS_TSX;
	static const int INS_TXA;
	static const int INS_TXS;
	static const int INS_TYA;
	static const int INS_DUMMY; // dummy instruction used for 'halting' the processor some cycles
	// -------------------------------- //
	// Addressing modes:
	static const int ADDR_ZP;
	static const int ADDR_REL;
	static const int ADDR_IMP;
	static const int ADDR_ABS;
	static const int ADDR_ACC;
	static const int ADDR_IMM;
	static const int ADDR_ZPX;
	static const int ADDR_ZPY;
	static const int ADDR_ABSX;
	static const int ADDR_ABSY;
	static const int ADDR_PREIDXIND;
	static const int ADDR_POSTIDXIND;
	static const int ADDR_INDABS;

	static array<string, 56> getInstNames();
	static string getInstName(size_t inst);
	static array<string, 13> getAddressModeNames();
	static string getAddressModeName(int addrMode);
	static void initOpData();
	static void setOp(int inst, int op, int addr, int size, int cycles);
};

class InputHandler : public enable_shared_from_this<InputHandler> {
public:
	static const float AXES_DEAD_ZONE;
	bool _is_gamepad_connected;
	string _gamepad_vendor_id;
	string _gamepad_product_id;
	bool _is_gamepad_used;
	bool _is_keyboard_used;
	map<string, bool> _is_input_pressed;
	map<string, vector<size_t> > _input_map_button;
	map<string, vector<size_t> > _input_map_axes_pos;
	map<string, vector<size_t> > _input_map_axes_neg;
	static const string KEYS[];
	static const size_t KEYS_LENGTH;
	static bool _is_configuring_gamepad;
	static string _configuring_gamepad_button;

	// Joypad keys:
	static const int KEY_A = 0;
	static const int KEY_B = 1;
	static const int KEY_START = 2;
	static const int KEY_SELECT = 3;
	static const int KEY_UP = 4;
	static const int KEY_DOWN = 5;
	static const int KEY_LEFT = 6;
	static const int KEY_RIGHT = 7;

	// Key count:
	static const int NUM_KEYS = 8;

	void key_down(uint32_t key) {
		_is_gamepad_used = false;
		_is_keyboard_used = true;
		switch(key) {
			case(38): _is_input_pressed["up"] = true; break; // up = 38
			case(37): _is_input_pressed["left"] = true; break; // left = 37
			case(40): _is_input_pressed["down"] = true; break; // down = 40
			case(39): _is_input_pressed["right"] = true; break; // right = 39
			case(13): _is_input_pressed["start"] = true; break; // enter = 13
			case(17): _is_input_pressed["select"] = true; break; // ctrl = 17
			case(90): _is_input_pressed["b"] = true; break; // z = 90
			case(88): _is_input_pressed["a"] = true; break; // x = 88
		}
	}
	void key_up(uint32_t key) {
		_is_gamepad_used = false;
		_is_keyboard_used = true;
		switch(key) {
			case(38): _is_input_pressed["up"] = false; break; // up = 38
			case(37): _is_input_pressed["left"] = false; break; // left = 37
			case(40): _is_input_pressed["down"] = false; break; // down = 40
			case(39): _is_input_pressed["right"] = false; break; // right = 39
			case(13): _is_input_pressed["start"] = false; break; // enter = 13
			case(17): _is_input_pressed["select"] = false; break; // ctrl = 17
			case(90): _is_input_pressed["b"] = false; break; // z = 90
			case(88): _is_input_pressed["a"] = false; break; // x = 88
		}
	}

	int _id;
	vector<bool> _keys;
	vector<int> _map;

	explicit InputHandler(int id);
	~InputHandler();
	uint16_t getKeyState(int padKey);
	void mapKey(int padKey, int kbKeycode);
	void poll_for_key_events();
	void reset();
};

class Logger {
	static string _file_name;
	static ofstream* _log;
	static size_t _length;
	static size_t _log_number;
	static bool _is_on;

public:
	static void init(string file_name);
	static void create_log_file();
	static void write(string message);
	static void flush();
};

class Memory : public enable_shared_from_this<Memory> {
public:
	shared_ptr<NES> nes;
	vector<uint16_t> mem;

	Memory();
	shared_ptr<Memory> Init(shared_ptr<NES> nes, size_t byteCount);
	~Memory();
	void stateLoad(ByteBuffer* buf);
	void stateSave(ByteBuffer* buf);
	void reset();
	size_t getMemSize();
	void write(size_t address, uint16_t value);
	uint16_t load(size_t address);
	void dump(string file);
	void dump(string file, size_t offset, size_t length);
	void write(size_t address, array<uint16_t, 16384>* array, size_t length);
	void write(size_t address, array<uint16_t, 16384>* array, size_t arrayoffset, size_t length);
};

class MapperDefault : public enable_shared_from_this<MapperDefault> {
public:
	shared_ptr<NES> nes;
	shared_ptr<Memory> cpuMem;
	shared_ptr<Memory> ppuMem;
	vector<uint16_t>* cpuMemArray;
	shared_ptr<ROM> rom;
	shared_ptr<CPU> cpu;
	shared_ptr<PPU> ppu;
	int cpuMemSize;
	int joy1StrobeState;
	int joy2StrobeState;
	int joypadLastWrite;
	bool mousePressed;
	bool gameGenieActive;
	int mouseX;
	int mouseY;
	int tmp;

	MapperDefault();
	shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	virtual ~MapperDefault();
	virtual void write(int address, uint16_t value);
	void base_init(shared_ptr<NES> nes);
	void stateLoad(ByteBuffer* buf);
	void stateSave(ByteBuffer* buf);
	void base_mapperInternalStateLoad(ByteBuffer* buf);
	void base_mapperInternalStateSave(ByteBuffer* buf);
	void setGameGenieState(bool enable);
	bool getGameGenieState();
	void base_write(int address, uint16_t value);
	void writelow(int address, uint16_t value);
	uint16_t base_load(int address);
	virtual uint16_t load(int address);
	uint16_t regLoad(int address);
	void regWrite(int address, uint16_t value);
	uint16_t joy1Read();
	uint16_t joy2Read();
	virtual void loadROM(shared_ptr<ROM> rom);
	void loadPRGROM();
	void loadCHRROM();
	void loadBatteryRam();
	void loadRomBank(int bank, int address);
	void loadVromBank(int bank, int address);
	void load32kRomBank(int bank, int address);
	void load8kVromBank(int bank4kStart, int address);
	void load1kVromBank(int bank1k, int address);
	void load2kVromBank(int bank2k, int address);
	void load8kRomBank(int bank8k, int address);
	virtual void clockIrqCounter();
	void latchAccess(int address);
	int syncV();
	int syncH(int scanline);
	void setMouseState(bool pressed, int x, int y);
	virtual void base_reset();
	void reset();
};

class Mapper001 : public MapperDefault {
public:
	// Register flags:

	// Register 0:
	int mirroring;
	int oneScreenMirroring;
	int prgSwitchingArea;
	int prgSwitchingSize;
	int vromSwitchingSize;

	// Register 1:
	int romSelectionReg0;

	// Register 2:
	int romSelectionReg1;

	// Register 3:
	int romBankSelect;

	// 5-bit buffer:
	int regBuffer;
	int regBufferCounter;

	Mapper001();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	void mapperInternalStateLoad(ByteBuffer* buf);
	void mapperInternalStateSave(ByteBuffer* buf);
	virtual void write(int address, uint16_t value);
	void setReg(int reg, int value);
	int getRegNumber(int address);
	virtual void loadROM(shared_ptr<ROM> rom);
	virtual void reset();
	void switchLowHighPrgRom(int oldSetting);
	void switch16to32();
	void switch32to16();
};

class Mapper002 : public MapperDefault {
public:
	Mapper002();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	virtual void write(int address, uint16_t value);
	virtual void loadROM(shared_ptr<ROM> rom);
};

class Mapper003 : public MapperDefault {
public:
	Mapper003();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	virtual void write(int address, uint16_t value);
};

class Mapper004 : public MapperDefault {
public:
	static const int CMD_SEL_2_1K_VROM_0000 = 0;
	static const int CMD_SEL_2_1K_VROM_0800 = 1;
	static const int CMD_SEL_1K_VROM_1000 = 2;
	static const int CMD_SEL_1K_VROM_1400 = 3;
	static const int CMD_SEL_1K_VROM_1800 = 4;
	static const int CMD_SEL_1K_VROM_1C00 = 5;
	static const int CMD_SEL_ROM_PAGE1 = 6;
	static const int CMD_SEL_ROM_PAGE2 = 7;
	int command;
	int prgAddressSelect;
	int chrAddressSelect;
	int pageNumber;
	int irqCounter;
	int irqLatchValue;
	int irqEnable;
	bool prgAddressChanged;

	Mapper004();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	void mapperInternalStateLoad(ByteBuffer* buf);
	void mapperInternalStateSave(ByteBuffer* buf);
	virtual void write(int address, uint16_t value);
	virtual void executeCommand(int cmd, int arg);
	virtual void loadROM(shared_ptr<ROM> rom);
	virtual void clockIrqCounter();
	virtual void reset();
};

class Mapper007 : public MapperDefault {
public:
	int currentOffset;
	int currentMirroring;
	vector<uint16_t> prgrom;

	Mapper007();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	virtual uint16_t load(int address);
	virtual void write(int address, uint16_t value);
	void mapperInternalStateLoad(ByteBuffer* buf);
	void mapperInternalStateSave(ByteBuffer* buf);
	virtual void reset();
};

class Mapper009 : public MapperDefault {
public:
	int latchLo;
	int latchHi;
	int latchLoVal1;
	int latchLoVal2;
	int latchHiVal1;
	int latchHiVal2;

	Mapper009();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	virtual void write(int address, uint16_t value);
	virtual void loadROM(shared_ptr<ROM> rom);
	virtual void latchAccess(int address);
	void mapperInternalStateLoad(ByteBuffer* buf);
	void mapperInternalStateSave(ByteBuffer* buf);
	virtual void reset();
};

class Mapper011 : public MapperDefault {
public:
	Mapper011();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	virtual void write(int address, uint16_t value);
};

class Mapper018 : public MapperDefault {
public:
	int irq_counter;
	int irq_latch;
	bool irq_enabled;
	int regs[11];
	int num_8k_banks;
	int patch;

	Mapper018();
	virtual shared_ptr<MapperDefault> Init(shared_ptr<NES> nes);
	void mapperInternalStateLoad(ByteBuffer* buf);
	void mapperInternalStateSave(ByteBuffer* buf);
	virtual void write(int address, short value);
	virtual void loadROM(ROM* rom);
	virtual int syncH(int scanline);
	virtual void reset();
};

class Misc {
public:
	static vector<float>* _rnd;
	static int nextRnd;
	static float rndret;

	static vector<float>* rnd();
	static string hex8(int i);
	static string hex16(int i);
	static string binN(int num, int N);
	static string bin8(int num);
	static string bin16(int num);
	static string binStr(uint32_t value, int bitcount);
	static string pad(string str, string padStr, int length);
	static float random();
	static string from_vector_to_hex_string(array<uint16_t, 0x2000>* data);
	static vector<uint16_t>* from_hex_string_to_vector(string data);
};

class NameTable {
public:
	string name;
	array<uint16_t, 32*32> tile;
	array<uint16_t, 32*32> attrib;
	int width;
	int height;

	NameTable();
	NameTable(string name);
	~NameTable();
	uint16_t getTileIndex(int x, int y);
	uint16_t getAttrib(int x, int y);
	void writeTileIndex(int index, int value);
	void writeAttrib(int index, int value);
	void stateSave(ByteBuffer* buf);
	void stateLoad(ByteBuffer* buf);
};

class NES : public enable_shared_from_this<NES> {
public:
	bool _is_paused;
	shared_ptr<CPU> cpu;
	shared_ptr<PPU> ppu;
	shared_ptr<PAPU> papu;
	shared_ptr<InputHandler> _joy1;
	shared_ptr<InputHandler> _joy2;
	shared_ptr<Memory> cpuMem;
	shared_ptr<Memory> ppuMem;
	shared_ptr<Memory> sprMem;
	shared_ptr<MapperDefault> memMapper;
	shared_ptr<PaletteTable> palTable;
	shared_ptr<ROM> rom;
	int cc;
	bool _isRunning;

	NES();
	shared_ptr<NES> Init(shared_ptr<InputHandler> joy1, shared_ptr<InputHandler> joy2);
	~NES();
	bool stateLoad(ByteBuffer* buf);
	void stateSave(ByteBuffer* buf);
	bool isRunning();
	void startEmulation();
	void stopEmulation();
	void clearCPUMemory();
	void dumpRomMemory(ofstream* writer);
	void dumpCPUMemory(ofstream* writer);
	void setGameGenieState(bool enable);
	shared_ptr<CPU> getCpu();
	shared_ptr<PPU> getPpu();
	shared_ptr<PAPU> getPapu();
	shared_ptr<Memory> getCpuMemory();
	shared_ptr<Memory> getPpuMemory();
	shared_ptr<Memory> getSprMemory();
	shared_ptr<ROM> getRom();
	shared_ptr<MapperDefault> getMemoryMapper();
	bool load_rom_from_data(string rom_name, vector<uint8_t>* data, array<uint16_t, 0x2000>* save_ram);
	void reset();
	void enableSound(bool enable);
//	void setFramerate(int rate);
};

class PaletteTable : public enable_shared_from_this<PaletteTable> {
public:
	static int curTable[64];
	static int origTable[64];
	static int emphTable[8][64];

	int currentEmph;
	int currentHue, currentSaturation, currentLightness, currentContrast;

	PaletteTable();
	shared_ptr<PaletteTable> Init();
	bool loadNTSCPalette();
	void makeTables();
	void setEmphasis(int emph);
	int getEntry(int yiq);
	int RGBtoHSL(int r, int g, int b);
	int RGBtoHSL(int rgb);
	int HSLtoRGB(int h, int s, int l);
	int HSLtoRGB(int hsl);
	int getHue(int hsl);
	int getSaturation(int hsl);
	int getLightness(int hsl);
	int getRed(int rgb);
	int getGreen(int rgb);
	int getBlue(int rgb);
	static void setRed(int* rgb, int r);
	static void setGreen(int* rgb, int g);
	static void setBlue(int* rgb, int b);
	int getRgb(int r, int g, int b);
	void updatePalette();
	void updatePalette(int hueAdd, int saturationAdd, int lightnessAdd, int contrastAdd);
	void loadDefaultPalette();
	void reset();
};

 class PAPU : public enable_shared_from_this<PAPU> {
 public:
	// Panning:
	static const uint8_t panning[];
	static const uint8_t lengthLookup[];

	static const uint16_t dmcFreqLookup[];
	static const uint16_t noiseWavelengthLookup[];

	mutable pthread_mutex_t _mutex;
	bool _is_muted;
	bool _is_running;
	shared_ptr<NES> nes;
	shared_ptr<Memory> cpuMem;
	ChannelSquare square1;
	ChannelSquare square2;
	ChannelTriangle triangle;
	ChannelNoise noise;
	ChannelDM dmc;
	array<int, 32 * 16> square_table;
	array<int, 204 * 16> tnd_table;
	vector<int> ismpbuffer;
	vector<uint8_t> sampleBuffer;
	bool ready_for_buffer_write;
	int frameIrqCounter;
	int frameIrqCounterMax;
	int initCounter;
	uint16_t channelEnableValue;
	uint8_t b1, b2, b3, b4;
	int bufferSize;
	int sampleRate;
	bool frameIrqEnabled;
	bool frameIrqActive;
	bool frameClockNow;
	bool startedPlaying;
	bool recordOutput;
	bool stereo;
	bool initingHardware;
	bool userEnableSquare1;
	bool userEnableSquare2;
	bool userEnableTriangle;
	bool userEnableNoise;
	bool userEnableDmc;
	int masterFrameCounter;
	int derivedFrameCounter;
	int countSequence;
	int sampleTimer;
	int frameTime;
	int sampleTimerMax;
	int sampleCount;
	int sampleValueL, sampleValueR;
	int triValue;
	int smpSquare1, smpSquare2, smpTriangle, smpNoise, smpDmc;
	int accCount;
	int sq_index, tnd_index;

	// DC removal vars:
	int prevSampleL, prevSampleR;
	int smpAccumL, smpAccumR;
	int smpDiffL, smpDiffR;

	// DAC range:
	int dacRange;
	int dcValue;

	// Master volume:
	int masterVolume;

	// Stereo positioning:
	int stereoPosLSquare1;
	int stereoPosLSquare2;
	int stereoPosLTriangle;
	int stereoPosLNoise;
	int stereoPosLDMC;
	int stereoPosRSquare1;
	int stereoPosRSquare2;
	int stereoPosRTriangle;
	int stereoPosRNoise;
	int stereoPosRDMC;
	int extraCycles;
	int maxCycles;

	int bufferIndex;

	void lock_mutex();
	void unlock_mutex();
	explicit PAPU();
	shared_ptr<PAPU> Init(shared_ptr<NES> nes);
	~PAPU();
	void stateLoad(ByteBuffer* buf);
	void stateSave(ByteBuffer* buf);
	void synchronized_start();
	shared_ptr<NES> getNes();
	uint16_t readReg();
	void writeReg(int address, uint16_t value);
	void resetCounter();
	void updateChannelEnable(int value);
	void clockFrameCounter(int nCycles);
	void accSample(int cycles);
	void frameCounterTick();
	void sample();
	void writeBuffer();
	void stop();
	int getSampleRate();
	void reset();
	int getLengthMax(int value);
	int getDmcFrequency(int value);
	int getNoiseWaveLength(int value);
	void synchronized_setSampleRate(int rate, bool restart);
	void synchronized_setStereo(bool s, bool restart);
	size_t getPapuBufferSize();
	void setChannelEnabled(int channel, bool value);
	void setMasterVolume(int value);
	void updateStereoPos();
	bool isRunning();
	int getMillisToAvailableAbove(int target_avail);
	int getBufferPos();
	void initDACtables();
};

class Tile {
public:
	// Tile data:
	array<int, 64> pix;
	int fbIndex;
	int tIndex;
	int x, y;
	int w, h;
	int incX, incY;
	int palIndex;
	int tpri;
	int c;
	bool initialized;
	array<bool, 8> opaque;

	Tile();
	void setBuffer(vector<uint16_t>* scanline);
	void setScanline(int sline, uint16_t b1, uint16_t b2);
	void renderSimple(int dx, int dy, vector<int>* fBuffer, int palAdd, int* palette);
	void renderSmall(int dx, int dy, vector<int>* buffer, int palAdd, int* palette);
	void render(int srcx1, int srcy1, int srcx2, int srcy2, int dx, int dy, array<int, 256 * 240>* fBuffer, int palAdd, array<int, 16>* palette, bool flipHorizontal, bool flipVertical, int pri, array<int, 256 * 240>* priTable);
	bool isTransparent(int x, int y);
	void dumpData(string file);
	void stateSave(ByteBuffer* buf);
	void stateLoad(ByteBuffer* buf);
};

class PPU : public enable_shared_from_this<PPU> {
public:
	shared_ptr<NES> nes;
	static const size_t UNDER_SCAN;
	int _zoom;
	struct timeval _frame_start;
	struct timeval _frame_end;
	double _ticks_since_second;
	uint32_t frameCounter;
	shared_ptr<Memory> ppuMem;
	shared_ptr<Memory> sprMem;
	// Rendering Options:
	bool showSpr0Hit;
	// Control Flags Register 1:
	int f_nmiOnVblank; // NMI on VBlank. 0=disable, 1=enable
	int f_spriteSize; // Sprite size. 0=8x8, 1=8x16
	int f_bgPatternTable; // Background Pattern Table address. 0=0x0000,1=0x1000
	int f_spPatternTable; // Sprite Pattern Table address. 0=0x0000,1=0x1000
	int f_addrInc; // PPU Address Increment. 0=1,1=32
	int f_nTblAddress; // Name Table Address. 0=0x2000,1=0x2400,2=0x2800,3=0x2C00
	// Control Flags Register 2:
	int f_color; // Background color. 0=black, 1=blue, 2=green, 4=red
	int f_spVisibility; // Sprite visibility. 0=not displayed,1=displayed
	int f_bgVisibility; // Background visibility. 0=Not Displayed,1=displayed
	int f_spClipping; // Sprite clipping. 0=Sprites invisible in left 8-pixel column,1=No clipping
	int f_bgClipping; // Background clipping. 0=BG invisible in left 8-pixel column, 1=No clipping
	int f_dispType; // Display type. 0=color, 1=monochrome
	// Status flags:
	int STATUS_VRAMWRITE;
	int STATUS_SLSPRITECOUNT;
	int STATUS_SPRITE0HIT;
	int STATUS_VBLANK;
	// VRAM I/O:
	int vramAddress;
	int vramTmpAddress;
	uint16_t vramBufferedReadValue;
	bool firstWrite; 		// VRAM/Scroll Hi/Lo latch
	array<int, 0x8000> vramMirrorTable; // Mirroring Lookup Table.
	int i;

	// SPR-RAM I/O:
	uint16_t sramAddress; // 8-bit only.

	// Counters:
	int cntFV;
	int cntV;
	int cntH;
	int cntVT;
	int cntHT;

	// Registers:
	int regFV;
	int regV;
	int regH;
	int regVT;
	int regHT;
	int regFH;
	int regS;

	// VBlank extension for PAL emulation:
	int vblankAdd;
	int curX;
	int scanline;
	int lastRenderedScanline;
	int mapperIrqCounter;
	// Sprite data:
	array<int, 64> sprX;				// X coordinate
	array<int, 64> sprY;				// Y coordinate
	array<int, 64> sprTile;			// Tile Index (into pattern table)
	array<int, 64> sprCol;			// Upper two bits of color
	array<bool, 64> vertFlip;		// Vertical Flip
	array<bool, 64> horiFlip;		// Horizontal Flip
	array<bool, 64> bgPriority;	// Background priority
	int spr0HitX;	// Sprite #0 hit X coordinate
	int spr0HitY;	// Sprite #0 hit Y coordinate
	bool hitSpr0;

	// Tiles:
	array<Tile, 512> ptTile;
	// Name table data:
	array<int, 4> ntable1;
	array<NameTable, 4> nameTable;
	int currentMirroring;

	// Palette data:
	array<int, 16> sprPalette;
	array<int, 16> imgPalette;
	// Misc:
	bool scanlineAlreadyRendered;
	bool requestEndFrame;
	bool nmiOk;
	int nmiCounter;
	uint16_t tmp;
	bool dummyCycleToggle;

	// Vars used when updating regs/address:
	int address, b1, b2;
	// Variables used when rendering:
	array<int, 32> attrib;
	array<int, 256 * 240> bgbuffer;
	array<int, 256 * 240> pixrendered;
	//vector<int> dummyPixPriTable;
	array<int, 64>* tpix;
	bool requestRenderAll;
	bool validTileData;
	int att;
	array<Tile*, 32> scantile;
	Tile* t;
	// These are temporary variables used in rendering and sound procedures.
	// Their states outside of those procedures can be ignored.
	int curNt;
	int destIndex;
	int x, y, sx;
	int si, ei;
	int tile;
	int col;
	int baseTile;
	int tscanoffset;
	int srcy1, srcy2;
	int bufferSize, available;
	int cycles;
	array<int, 256 * 240> _screen_buffer;

	array<int, 256 * 240>* get_screen_buffer();
	vector<int>* get_pattern_buffer();
	vector<int>* get_name_buffer();
	vector<int>* get_img_palette_buffer();
	vector<int>* get_spr_palette_buffer();
	explicit PPU();
	shared_ptr<PPU> Init(shared_ptr<NES> nes);
	~PPU();
	void init();
	void setMirroring(int mirroring);
	void defineMirrorRegion(size_t fromStart, size_t toStart, size_t size);
	bool emulateCycles();
	void startVBlank();
	void endScanline();
	void startFrame();
	void endFrame();
	void updateControlReg1(int value);
	void updateControlReg2(int value);
	void setStatusFlag(int flag, bool value);
	uint16_t readStatusRegister();
	void writeSRAMAddress(uint16_t address);
	uint16_t sramLoad();
	void sramWrite(uint16_t value);
	void scrollWrite(uint16_t value);
	void writeVRAMAddress(int address);
	uint16_t vramLoad();
	void vramWrite(uint16_t value);
	void sramDMA(uint16_t value);
	void regsFromAddress();
	void cntsFromAddress();
	void regsToAddress();
	void cntsToAddress();
	void incTileCounter(int count);
	uint16_t mirroredLoad(int address);
	void mirroredWrite(int address, uint16_t value);
	void triggerRendering();
	void renderFramePartially(int startScan, int scanCount);
	void renderBgScanline(array<int, 256 * 240>* buffer, int scan);
	void renderSpritesPartially(int startscan, int scancount, bool bgPri);
	bool checkSprite0(int scan);
	void renderPattern();
	void renderNameTables();
	void renderPalettes();
	void writeMem(int address, uint16_t value);
	void updatePalettes();
	void patternWrite(int address, uint16_t value);
	void patternWrite(int address, vector<uint16_t>* value, size_t offset, size_t length);
	void invalidateFrameCache();
	void nameTableWrite(int index, int address, uint16_t value);
	void attribTableWrite(int index, int address, uint16_t value);
	void spriteRamWriteUpdate(int address, uint16_t value);
	void doNMI();
	int statusRegsToInt();
	void statusRegsFromInt(int n);
	void stateLoad(ByteBuffer* buf);
	void stateSave(ByteBuffer* buf);
	void reset();
};

class Raster {
public:
	vector<int>* data;
	int width;
	int height;

	Raster(vector<int>* data, int w, int h);
	Raster(int w, int h);
	void drawTile(Raster* srcRaster, int srcx, int srcy, int dstx, int dsty, int w, int h);
};

class MapperStatus {
public:
	int number;
	bool is_supported;
	string name;

	MapperStatus(int number, bool is_supported, string name) {
		this->number = number;
		this->is_supported = is_supported;
		this->name = name;
	}
};

class ROM : public enable_shared_from_this<ROM> {
public:
	// Mirroring types:
	static const int VERTICAL_MIRRORING = 0;
	static const int HORIZONTAL_MIRRORING = 1;
	static const int FOURSCREEN_MIRRORING = 2;
	static const int SINGLESCREEN_MIRRORING = 3;
	static const int SINGLESCREEN_MIRRORING2 = 4;
	static const int SINGLESCREEN_MIRRORING3 = 5;
	static const int SINGLESCREEN_MIRRORING4 = 6;
	static const int CHRROM_MIRRORING = 7;
	static const array<MapperStatus, 255> _mapperStatus;

	bool failedSaveFile;
	bool saveRamUpToDate;
	array<uint16_t, 16> header;
	vector<array<uint16_t, 16384>> rom;
	vector<array<uint16_t, 4096>> vrom;
	array<uint16_t, 0x2000>* saveRam;
	vector<array<Tile, 256>> vromTile;
	shared_ptr<NES> nes;
	size_t romCount;
	size_t vromCount;
	int mirroring;
	bool batteryRam;
	bool trainer;
	bool fourScreen;
	size_t mapperType;
	string fileName;
	string _sha256;
	bool enableSave;
	bool valid;

	explicit ROM();
	shared_ptr<ROM> Init(shared_ptr<NES> nes);
	~ROM();
	string sha256sum(uint8_t* data, size_t length);
	string getmapperName();
	void load_from_data(string file_name, vector<uint8_t>* data, array<uint16_t, 0x2000>* save_ram);
	bool isValid();
	int getRomBankCount();
	int getVromBankCount();
	array<uint16_t, 16> getHeader();
	array<uint16_t, 16384>* getRomBank(int bank);
	array<uint16_t, 4096>* getVromBank(int bank);
	array<Tile, 256>* getVromBankTiles(int bank);
	int getMirroringType();
	size_t getMapperType();
	string getMapperName();
	bool hasBatteryRam();
	bool hasTrainer();
	string getFileName();
	bool mapperSupported();
	shared_ptr<MapperDefault> createMapper();
	void setSaveState(bool enableSave);
	array<uint16_t, 0x2000>* getBatteryRam();
	void loadBatteryRam();
	void writeBatteryRam(int address, uint16_t value);
	void closeRom();
};

class SaltyNES {
public:
	int samplerate;
	int progress;
	shared_ptr<NES> nes;
	string _rom_name;

	SaltyNES();
	~SaltyNES();
	void init();
	void load_rom(string rom_name, vector<uint8_t>* rom_data, array<uint16_t, 0x2000>* save_ram);
	void run();
	void stop();
	void readParams();
	void initKeyCodes();
};

template<typename SRC, typename DST> inline void array_copy(SRC src, size_t srcPos, DST dest, size_t destPos, size_t length) {
	assert(srcPos+length <= src->size());
	assert(destPos+length <= dest->size());

	std::copy(src->begin()+srcPos, src->begin()+srcPos+length, dest->begin()+destPos);
}

inline string intToHexString(int i) {
	stringstream ss;
	ss << std::hex << std::showbase << i;
	return ss.str();
}


inline string toUpperCase(string s) {
	std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))std::toupper);
	return s;
}

inline string toLowerCase(string s) {
	std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))std::tolower);
	return s;
}

inline bool endsWith(string str, string key) {
	size_t keylen = key.length();
	size_t strlen = str.length();

	if(keylen == 0 && strlen == 0) {
		return true;
	} else if(keylen == 0 || strlen == 0) {
		return false;
	}

	if(keylen <= strlen && str.substr(strlen - keylen, keylen) == key) {
		return true;
	} else {
		return false;
	}
}

inline bool startsWith(string str, string key) {
	size_t keylen = key.length();
	size_t strlen = str.length();

	if(keylen == 0 && strlen == 0) {
		return true;
	} else if(keylen == 0 || strlen == 0) {
		return false;
	}

	if(keylen <= strlen && str.substr(0, keylen) == key) {
		return true;
	} else {
		return false;
	}
}

inline float rand_float() {
	return static_cast<float>(rand()) / RAND_MAX;
}

template <typename T>
inline T hexStringTo(string str) {
	T x;
	std::stringstream ss;
	ss << std::hex << str;
	ss >> x;
	return x;
}

inline uint8_t string_to_half_byte(uint8_t byte) {
	// '0'(48) through '9'(57)
	if(byte >= 48 && byte <= 57) {
		byte -= 48;
	// 'A'(65) through 'F'(70)
	} else if(byte >= 65 && byte <= 70) {
		byte -= 55;
	}
	return byte;
}

inline uint8_t string_to_byte(uint8_t upper, uint8_t lower) {
	uint8_t retval = 0;
	upper = string_to_half_byte(upper);
	lower = string_to_half_byte(lower);
	retval = static_cast<uint8_t>(upper << 4) | lower;
	return retval;
}

inline void log_to_browser(string message) {
	fprintf(stdout, "%s\n", message.c_str());
	fflush(stdout);
}

#endif // _SALTY_NES_H_
