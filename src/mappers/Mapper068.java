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

public class Mapper068 extends MapperDefault {

    int r1, r2, r3, r4;

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);
            return;
        }

        switch ((address >> 12) - 0x8) {

            case 0: {

                // Select 2K VROM bank at 0x0000
                load2kVromBank(value, 0x0000);
                break;

            }

            case 1: {

                // Select 2K VROM bank at 0x0800
                load2kVromBank(value, 0x0800);
                break;

            }

            case 2: {

                // Select 2K VROM bank at 0x1000
                load2kVromBank(value, 0x1000);
                break;

            }

            case 3: {

                // Select 2K VROM bank at 0x1800
                load2kVromBank(value, 0x1800);
                break;

            }

            case 4: {

                // Mirroring.
                r3 = value;
                setMirroring();
                break;

            }

            case 5: {

                // Mirroring.
                r4 = value;
                setMirroring();
                break;

            }

            case 6: {

                // Mirroring.
                r1 = (value >> 4) & 0x1;
                r2 = value & 0x3;
                setMirroring();
                break;

            }

            case 7: {

                // Select 16K ROM bank at 0x8000
                loadRomBank(value, 0x8000);
                break;

            }

        }

    }

    private void setMirroring() {

        if (r1 == 0) {

            // Normal mirroring modes:
            switch (r2) {
                case 0: {
                    ppu.setMirroring(ROM.HORIZONTAL_MIRRORING);
                    break;
                }
                case 1: {
                    ppu.setMirroring(ROM.VERTICAL_MIRRORING);
                    break;
                }
                case 2: {
                    ppu.setMirroring(ROM.SINGLESCREEN_MIRRORING);
                    break;
                }
                case 3: {
                    ppu.setMirroring(ROM.SINGLESCREEN_MIRRORING2);
                    break;
                }
            }

        } else {

            // Special mirroring (not yet..):
            switch (r2) {
                case 0: {
                    break;
                }
                case 1: {
                    break;
                }
                case 2: {
                    break;
                }
                case 3: {
                    break;
                }
            }

        }

    }

    public void loadROM(ROM rom) {

        //System.out.println("Loading ROM.");

        if (!rom.isValid()) {
            //System.out.println("Sunsoft#4: Invalid ROM! Unable to load.");
            return;
        }

        // Get number of PRG ROM banks:
        int num_banks = rom.getRomBankCount();

        // Load PRG-ROM:
        loadRomBank(0, 0x8000);
        loadRomBank(num_banks - 1, 0xC000);

        // Load CHR-ROM:
        loadCHRROM();

        // Load Battery RAM (if present):
        loadBatteryRam();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);

    }

    public void reset() {

        r1 = r2 = r3 = r4 = 0;

    }
}