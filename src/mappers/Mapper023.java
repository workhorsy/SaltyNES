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

public class Mapper023 extends MapperDefault {

    private int irq_counter = 0;
    private int irq_latch = 0;
    private int irq_enabled = 0;
    private int regs[] = new int[9];
    int patch = 0xFFFF;

    public void init(NES nes) {
        super.init(nes);
        reset();
    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);
        } else {
            switch (address & patch) {
                case 0x8000:
                case 0x8004:
                case 0x8008:
                case 0x800C:
                     {
                        if ((regs[8]) != 0) {
                            load8kRomBank(value, 0xC000);
                        } else {
                            load8kRomBank(value, 0x8000);
                        }
                    }
                    break;

                case 0x9000:
                     {
                        if (value != 0xFF) {
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
                    }
                    break;

                case 0x9008:
                     {
                        regs[8] = value & 0x02;
                    }
                    break;

                case 0xA000:
                case 0xA004:
                case 0xA008:
                case 0xA00C:
                     {
                        load8kRomBank(value, 0xA000);
                    }
                    break;

                case 0xB000:
                     {
                        regs[0] = (regs[0] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[0], 0x0000);
                    }
                    break;

                case 0xB001:
                case 0xB004:
                     {
                        regs[0] = (regs[0] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[0], 0x0000);
                    }
                    break;

                case 0xB002:
                case 0xB008:
                     {
                        regs[1] = (regs[1] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[1], 0x0400);
                    }
                    break;

                case 0xB003:
                case 0xB00C:
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

                case 0xC001:
                case 0xC004:
                     {
                        regs[2] = (regs[2] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[2], 0x0800);
                    }
                    break;

                case 0xC002:
                case 0xC008:
                     {
                        regs[3] = (regs[3] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[3], 0x0C00);
                    }
                    break;

                case 0xC003:
                case 0xC00C:
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

                case 0xD001:
                case 0xD004:
                     {
                        regs[4] = (regs[4] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[4], 0x1000);
                    }
                    break;

                case 0xD002:
                case 0xD008:
                     {
                        regs[5] = (regs[5] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[5], 0x1400);
                    }
                    break;

                case 0xD003:
                case 0xD00C:
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

                case 0xE001:
                case 0xE004:
                     {
                        regs[6] = (regs[6] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[6], 0x1800);
                    }
                    break;

                case 0xE002:
                case 0xE008:
                     {
                        regs[7] = (regs[7] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[7], 0x1C00);
                    }
                    break;

                case 0xE003:
                case 0xE00C:
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

                case 0xF004:
                     {
                        irq_latch = (irq_latch & 0x0F) | ((value & 0x0F) << 4);
                    }
                    break;

                case 0xF008:
                     {
                        irq_enabled = value & 0x03;
                        if ((irq_enabled & 0x02) != 0) {
                            irq_counter = irq_latch;
                        }
                    }
                    break;

                case 0xF00C:
                     {
                        irq_enabled = (irq_enabled & 0x01) * 3;
                    }
                    break;
            }
        }
    }

    public void loadROM(ROM rom) {

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

    public int syncH(int scanline) {


        if ((irq_enabled & 0x02) != 0) {
            if (irq_counter == 0xFF) {
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