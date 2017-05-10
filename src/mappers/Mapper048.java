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

public class Mapper048 extends MapperDefault {

    private int irq_counter = 0;
    private boolean irq_enabled = false;

    public void init(NES nes) {
        super.init(nes);
        reset();
    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);
        } else {

            switch (address) {
                case 0x8000:
                     {
                        load8kRomBank(value, 0x8000);
                    }
                    break;

                case 0x8001:
                     {
                        load8kRomBank(value, 0xA000);
                    }
                    break;

                case 0x8002:
                     {
                        load2kVromBank(value * 2, 0x0000);
                    }
                    break;

                case 0x8003:
                     {
                        load2kVromBank(value * 2, 0x0800);
                    }
                    break;

                case 0xA000:
                     {
                        load1kVromBank(value, 0x1000);
                    }
                    break;

                case 0xA001:
                     {
                        load1kVromBank(value, 0x1400);
                    }
                    break;

                case 0xA002:
                     {
                        load1kVromBank(value, 0x1800);
                    }
                    break;

                case 0xA003:
                     {
                        load1kVromBank(value, 0x1C00);
                    }
                    break;

                case 0xC000:
                     {
                        irq_counter = value;
                    }
                    break;

                case 0xC001:
                case 0xC002:
                case 0xE001:
                case 0xE002:
                     {
                        irq_enabled = (value != 0);
                    }
                    break;

                case 0xE000:
                     {
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
            System.out.println("VRC4: Invalid ROM! Unable to load.");
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

        // Load Battery RAM (if present):
        loadBatteryRam();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);
    }

    public int syncH(int scanline) {
        if (irq_enabled) {
            if ((ppu.scanline & 0x18) != 00) {
                if (scanline >= 0 && scanline <= 239) {
                    if (irq_counter == 0) {
                        irq_counter = 0;
                        irq_enabled = false;

                        return 3;

                    } else {
                        irq_counter++;
                    }
                }
            }
        }

        return 0;
    }

    public void reset() {

        irq_enabled = false;
        irq_counter = 0;

    }
}
