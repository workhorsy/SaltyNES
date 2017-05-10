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

public class Mapper022 extends MapperDefault {

    public void init(NES nes) {

        super.init(nes);
        reset();

    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);

        } else {
            //VRC2 write.
            switch (address) {
                case 0x8000:
                     {
                        load8kRomBank(value, 0x8000);
                    }
                    break;
                case 0x9000:
                     {
                        value &= 0x03;
                        if (value == 0) {
                            nes.getPpu().setMirroring(ROM.VERTICAL_MIRRORING);
                        } else if (value == 1) {
                            nes.getPpu().setMirroring(ROM.HORIZONTAL_MIRRORING);
                        } else if (value == 2) {
                            nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING);
                        } else {
                            nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING2);
                        }
                    }
                    break;
                case 0xA000:
                     {
                        load8kRomBank(value, 0xA000);
                    }
                    break;
                case 0xB000:
                     {
                        load1kVromBank((value >> 1), 0x0000);
                    }
                    break;
                case 0xB001:
                     {
                        load1kVromBank((value >> 1), 0x0400);
                    }
                    break;
                case 0xC000:
                     {
                        load1kVromBank((value >> 1), 0x0800);
                    }
                    break;
                case 0xC001:
                     {
                        load1kVromBank((value >> 1), 0x0C00);
                    }
                    break;
                case 0xD000:
                     {
                        load1kVromBank((value >> 1), 0x1000);
                    }
                    break;
                case 0xD001:
                     {
                        load1kVromBank((value >> 1), 0x1400);
                    }
                    break;
                case 0xE000:
                     {
                        load1kVromBank((value >> 1), 0x1800);
                    }
                    break;
                case 0xE001:
                     {
                        load1kVromBank((value >> 1), 0x1C00);
                    }
                    break;
            }
        }

    }

    public void loadROM(ROM rom) {

        //System.out.println("Loading ROM.");

        if (!rom.isValid()) {
            System.out.println("VRC2: Invalid ROM! Unable to load.");
            return;
        }

        // Get number of 8K banks:
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
}
