/*
vNES
Copyright © 2006-2011 Jamie Sanders

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program.  If not, see <http://www.gnu.org/licenses/>.
 */

public class Mapper015 extends MapperDefault {

    public void init(NES nes) {
        super.init(nes);
    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);
        } else {
            switch (address) {

                case 0x8000:
                     {
                        if ((value & 0x80) != 0) {
                            load8kRomBank((value & 0x3F) * 2 + 1, 0x8000);
                            load8kRomBank((value & 0x3F) * 2 + 0, 0xA000);
                            load8kRomBank((value & 0x3F) * 2 + 3, 0xC000);
                            load8kRomBank((value & 0x3F) * 2 + 2, 0xE000);
                        } else {
                            load8kRomBank((value & 0x3F) * 2 + 0, 0x8000);
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xA000);
                            load8kRomBank((value & 0x3F) * 2 + 2, 0xC000);
                            load8kRomBank((value & 0x3F) * 2 + 3, 0xE000);
                        }
                        if ((value & 0x40) != 0) {
                            nes.getPpu().setMirroring(ROM.HORIZONTAL_MIRRORING);
                        } else {
                            nes.getPpu().setMirroring(ROM.VERTICAL_MIRRORING);
                        }
                    }
                    break;
                case 0x8001:
                     {
                        if ((value & 0x80) != 0) {
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xC000);
                            load8kRomBank((value & 0x3F) * 2 + 0, 0xE000);
                        } else {
                            load8kRomBank((value & 0x3F) * 2 + 0, 0xC000);
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xE000);
                        }
                    }
                    break;
                case 0x8002:
                     {
                        if ((value & 0x80) != 0) {
                            load8kRomBank((value & 0x3F) * 2 + 1, 0x8000);
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xA000);
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xC000);
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xE000);
                        } else {
                            load8kRomBank((value & 0x3F) * 2, 0x8000);
                            load8kRomBank((value & 0x3F) * 2, 0xA000);
                            load8kRomBank((value & 0x3F) * 2, 0xC000);
                            load8kRomBank((value & 0x3F) * 2, 0xE000);
                        }
                    }
                    break;
                case 0x8003:
                     {
                        if ((value & 0x80) != 0) {
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xC000);
                            load8kRomBank((value & 0x3F) * 2 + 0, 0xE000);
                        } else {
                            load8kRomBank((value & 0x3F) * 2 + 0, 0xC000);
                            load8kRomBank((value & 0x3F) * 2 + 1, 0xE000);
                        }
                        if ((value & 0x40) != 0) {
                            nes.getPpu().setMirroring(ROM.HORIZONTAL_MIRRORING);
                        } else {
                            nes.getPpu().setMirroring(ROM.VERTICAL_MIRRORING);
                        }
                    }
                    break;
            }
        }
    }

    public void loadROM(ROM rom) {

        if (!rom.isValid()) {
            System.out.println("015: Invalid ROM! Unable to load.");
            return;

        }

        // Load PRG-ROM:
        load8kRomBank(0, 0x8000);
        load8kRomBank(1, 0xA000);
        load8kRomBank(2, 0xC000);
        load8kRomBank(3, 0xE000);

        // Load CHR-ROM:
        loadCHRROM();

        // Load Battery RAM (if present):
        loadBatteryRam();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);
    }
}