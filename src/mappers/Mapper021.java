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

public class Mapper021 extends MapperDefault {

    private int irq_counter = 0;
    private int irq_latch = 0;
    private int irq_enabled = 0;
    private int regs[] = new int[9];

    public void init(NES nes) {
        super.init(nes);
        reset();
    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);
        } else {
            switch (address & 0xF0CF) {
                case 0x8000:
                     {
                        if ((regs[8] & 0x02) != 0) {
                            load8kRomBank(value, 0xC000);
                        } else {
                            load8kRomBank(value, 0x8000);
                        }
                    }
                    break;

                case 0xA000:
                     {
                        load8kRomBank(value, 0xA000);
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

                case 0x9002:
                case 0x9080:
                     {
                        regs[8] = value;
                    }
                    break;

                case 0xB000:
                     {
                        regs[0] = (regs[0] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[0], 0x0000);
                    }
                    break;

                case 0xB002:
                case 0xB040:
                     {
                        regs[0] = (regs[0] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[0], 0x0000);
                    }
                    break;

                case 0xB001:
                case 0xB004:
                case 0xB080:
                     {
                        regs[1] = (regs[1] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[1], 0x0400);
                    }
                    break;

                case 0xB003:
                case 0xB006:
                case 0xB0C0:
                     {
                        regs[1] = (regs[1] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[1], 0x0400);
                    }
                    break;

                case 0xC000:
                     {
                        regs[2] = (regs[2] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[2], 0x0800);
                    }
                    break;

                case 0xC002:
                case 0xC040:
                     {
                        regs[2] = (regs[2] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[2], 0x0800);
                    }
                    break;

                case 0xC001:
                case 0xC004:
                case 0xC080:
                     {
                        regs[3] = (regs[3] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[3], 0x0C00);
                    }
                    break;

                case 0xC003:
                case 0xC006:
                case 0xC0C0:
                     {
                        regs[3] = (regs[3] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[3], 0x0C00);
                    }
                    break;

                case 0xD000:
                     {
                        regs[4] = (regs[4] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[4], 0x1000);
                    }
                    break;

                case 0xD002:
                case 0xD040:
                     {
                        regs[4] = (regs[4] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[4], 0x1000);
                    }
                    break;

                case 0xD001:
                case 0xD004:
                case 0xD080:
                     {
                        regs[5] = (regs[5] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[5], 0x1400);
                    }
                    break;

                case 0xD003:
                case 0xD006:
                case 0xD0C0:
                     {
                        regs[5] = (regs[5] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[5], 0x1400);
                    }
                    break;

                case 0xE000:
                     {
                        regs[6] = (regs[6] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[6], 0x1800);
                    }
                    break;

                case 0xE002:
                case 0xE040:
                     {
                        regs[6] = (regs[6] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[6], 0x1800);
                    }
                    break;

                case 0xE001:
                case 0xE004:
                case 0xE080:
                     {
                        regs[7] = (regs[7] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[7], 0x1C00);
                    }
                    break;

                case 0xE003:
                case 0xE006:
                case 0xE0C0:
                     {
                        regs[7] = (regs[7] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[7], 0x1C00);
                    }
                    break;

                case 0xF000:
                     {
                        irq_latch = (irq_latch & 0xF0) | (value & 0x0F);
                    }
                    break;

                case 0xF002:
                case 0xF040:
                     {
                        irq_latch = (irq_latch & 0x0F) | ((value & 0x0F) << 4);
                    }
                    break;

                case 0xF003:
                case 0xF0C0:
                     {
                        irq_enabled = (irq_enabled & 0x01) * 3;
                    }
                    break;

                case 0xF004:
                case 0xF080:
                     {
                        irq_enabled = value & 0x03;
                        if ((irq_enabled & 0x02) != 0) {
                            irq_counter = irq_latch;
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

        if ((irq_enabled & 0x02) != 0) {
            if (irq_counter == 0) {
                irq_counter = irq_latch;
                irq_enabled = (irq_enabled & 0x01) * 3;
                return 3;
            } else {
                irq_counter++;
            }
        }

        return 0;

    }

    public void reset() {

        regs[0] = 0;
        regs[1] = 1;
        regs[2] = 2;
        regs[3] = 3;
        regs[4] = 4;
        regs[5] = 5;
        regs[6] = 6;
        regs[7] = 7;
        regs[8] = 0;

        // IRQ Settings
        irq_enabled = 0;
        irq_latch = 0;
        irq_counter = 0;
    }
}