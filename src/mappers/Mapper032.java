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

public class Mapper032 extends MapperDefault {

    int regs[] = new int[1];
    int patch = 0;

    public void init(NES nes) {
        super.init(nes);
    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);
        } else {

            switch (address & 0xF000) {
                case 0x8000:
                     {
                        if ((regs[0] & 0x02) != 0) {
                            load8kRomBank(value, 0xC000);
                        } else {
                            load8kRomBank(value, 0x8000);
                        }
                    }
                    break;

                case 0x9000:
                     {
                        if ((value & 0x01) != 0) {
                            nes.getPpu().setMirroring(ROM.HORIZONTAL_MIRRORING);
                        } else {
                            nes.getPpu().setMirroring(ROM.VERTICAL_MIRRORING);
                        }
                        regs[0] = value;
                    }
                    break;

                case 0xA000:
                     {
                        load8kRomBank(value, 0xA000);
                    }
                    break;
            }


            switch (address & 0xF007) {
                case 0xB000:
                     {
                        load1kVromBank(value, 0x0000);
                    }
                    break;

                case 0xB001:
                     {
                        load1kVromBank(value, 0x0400);
                    }
                    break;

                case 0xB002:
                     {
                        load1kVromBank(value, 0x0800);
                    }
                    break;

                case 0xB003:
                     {
                        load1kVromBank(value, 0x0C00);
                    }
                    break;

                case 0xB004:
                     {
                        load1kVromBank(value, 0x1000);
                    }
                    break;

                case 0xB005:
                     {
                        load1kVromBank(value, 0x1400);
                    }
                    break;

                case 0xB006:
                     {
                        if ((patch == 1) && ((value & 0x40) != 0)) {
                            // nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING); /* 0,0,0,1 */
                        }
                        load1kVromBank(value, 0x1800);
                    }
                    break;

                case 0xB007:
                     {
                        if ((patch == 1) && ((value & 0x40) != 0)) {
                            nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING);
                        }
                        load1kVromBank(value, 0x1C00);
                    }
                    break;
            }
        }
    }

    public void loadROM(ROM rom) {

        int num_8k_banks = rom.getRomBankCount() * 2;

        // Load PRG-ROM:
        load8kRomBank(0, 0x8000);
        load8kRomBank(1, 0xA000);
        load8kRomBank(num_8k_banks - 2, 0xC000);
        load8kRomBank(num_8k_banks - 1, 0xE000);

        // Load CHR-ROM:
        loadCHRROM();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);

    }

    public void reset() {

        if (patch == 1) {
            nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING);
        }

        for (int i = 0; i < regs.length; i++) {
            regs[i] = 0;
        }
    }
}
