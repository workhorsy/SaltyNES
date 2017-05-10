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

public class Mapper018 extends MapperDefault {

    private int irq_counter = 0;
    private int irq_latch = 0;
    private boolean irq_enabled = false;
    private int regs[] = new int[11];
    int num_8k_banks;
    int patch = 0;

    public void init(NES nes) {

        super.init(nes);
        reset();

    }

    public void mapperInternalStateLoad(ByteBuffer buf) {
        super.mapperInternalStateLoad(buf);

        if (buf.readByte() == 1) {
            irq_counter = buf.readInt();
            irq_latch = buf.readInt();
            irq_enabled = buf.readBoolean();
        }
    }

    public void mapperInternalStateSave(ByteBuffer buf) {
        super.mapperInternalStateLoad(buf);

        // Version:
        buf.putByte((short) 1);

        buf.putInt(irq_counter);
        buf.putInt(irq_latch);
        buf.putBoolean(irq_enabled);
    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);

        } else {

            switch (address) {
                case 0x8000:
                     {
                        regs[0] = (regs[0] & 0xF0) | (value & 0x0F);
                        load8kRomBank(regs[0], 0x8000);
                    }
                    break;

                case 0x8001:
                     {
                        regs[0] = (regs[0] & 0x0F) | ((value & 0x0F) << 4);
                        load8kRomBank(regs[0], 0x8000);
                    }
                    break;

                case 0x8002:
                     {
                        regs[1] = (regs[1] & 0xF0) | (value & 0x0F);
                        load8kRomBank(regs[1], 0xA000);
                    }
                    break;

                case 0x8003:
                     {
                        regs[1] = (regs[1] & 0x0F) | ((value & 0x0F) << 4);
                        load8kRomBank(regs[1], 0xA000);
                    }
                    break;

                case 0x9000:
                     {
                        regs[2] = (regs[2] & 0xF0) | (value & 0x0F);
                        load8kRomBank(regs[2], 0xC000);
                    }
                    break;

                case 0x9001:
                     {
                        regs[2] = (regs[2] & 0x0F) | ((value & 0x0F) << 4);
                        load8kRomBank(regs[2], 0xC000);
                    }
                    break;

                case 0xA000:
                     {
                        regs[3] = (regs[3] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[3], 0x0000);
                    }
                    break;

                case 0xA001:
                     {
                        regs[3] = (regs[3] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[3], 0x0000);
                    }
                    break;

                case 0xA002:
                     {
                        regs[4] = (regs[4] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[4], 0x0400);
                    }
                    break;

                case 0xA003:
                     {
                        regs[4] = (regs[4] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[4], 0x0400);
                    }
                    break;

                case 0xB000:
                     {
                        regs[5] = (regs[5] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[5], 0x0800);
                    }
                    break;

                case 0xB001:
                     {
                        regs[5] = (regs[5] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[5], 0x0800);
                    }
                    break;

                case 0xB002:
                     {
                        regs[6] = (regs[6] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[6], 0x0C00);
                    }
                    break;

                case 0xB003:
                     {
                        regs[6] = (regs[6] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[6], 0x0C00);
                    }
                    break;

                case 0xC000:
                     {
                        regs[7] = (regs[7] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[7], 0x1000);
                    }
                    break;

                case 0xC001:
                     {
                        regs[7] = (regs[7] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[7], 0x1000);
                    }
                    break;

                case 0xC002:
                     {
                        regs[8] = (regs[8] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[8], 0x1400);
                    }
                    break;

                case 0xC003:
                     {
                        regs[8] = (regs[8] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[8], 0x1400);
                    }
                    break;

                case 0xD000:
                     {
                        regs[9] = (regs[9] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[9], 0x1800);
                    }
                    break;

                case 0xD001:
                     {
                        regs[9] = (regs[9] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[9], 0x1800);
                    }
                    break;

                case 0xD002:
                     {
                        regs[10] = (regs[10] & 0xF0) | (value & 0x0F);
                        load1kVromBank(regs[10], 0x1C00);
                    }
                    break;

                case 0xD003:
                     {
                        regs[10] = (regs[10] & 0x0F) | ((value & 0x0F) << 4);
                        load1kVromBank(regs[10], 0x1C00);
                    }
                    break;

                case 0xE000:
                     {
                        irq_latch = (irq_latch & 0xFFF0) | (value & 0x0F);
                    }
                    break;

                case 0xE001:
                     {
                        irq_latch = (irq_latch & 0xFF0F) | ((value & 0x0F) << 4);
                    }
                    break;

                case 0xE002:
                     {
                        irq_latch = (irq_latch & 0xF0FF) | ((value & 0x0F) << 8);
                    }
                    break;

                case 0xE003:
                     {
                        irq_latch = (irq_latch & 0x0FFF) | ((value & 0x0F) << 12);
                    }
                    break;

                case 0xF000:
                     {
                        irq_counter = irq_latch;
                    }
                    break;

                case 0xF001:
                     {
                        irq_enabled = (value & 0x01) != 0;
                    }
                    break;

                case 0xF002:
                     {
                        value &= 0x03;

                        if (value == 0) {
                            nes.getPpu().setMirroring(ROM.HORIZONTAL_MIRRORING);
                        } else if (value == 1) {
                            nes.getPpu().setMirroring(ROM.VERTICAL_MIRRORING);
                        } else {
                            nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING);
                        }

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

        loadBatteryRam();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);

    }

    public int syncH(int scanline) {

        if (irq_enabled) {
            if (irq_counter <= 113) {

                irq_counter = (patch == 1) ? 114 : 0;
                irq_enabled = false;
                return 3;
            } else {
                irq_counter -= 113;
            }
        }

        return 0;

    }

    public void reset() {

        regs[0] = 0;
        regs[1] = 1;
        regs[2] = num_8k_banks - 2;
        regs[3] = num_8k_banks - 1;
        regs[4] = 0;
        regs[5] = 0;
        regs[6] = 0;
        regs[7] = 0;
        regs[8] = 0;
        regs[9] = 0;
        regs[10] = 0;

        // IRQ Settings
        irq_enabled = false;
        irq_latch = 0;
        irq_counter = 0;
    }
}
