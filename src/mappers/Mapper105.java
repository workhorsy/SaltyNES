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

public class Mapper105 extends MapperDefault {

    private int irq_counter = 0;
    private boolean irq_enabled = false;
    private int init_state = 0;
    private int[] regs = new int[4];
    int bits = 0;
    int write_count = 0;

    public void init(NES nes) {
        super.init(nes);
        reset();
    }

    public void mapperInternalStateLoad(ByteBuffer buf) {
        super.mapperInternalStateLoad(buf);

        if (buf.readByte() == 1) {
            irq_counter = buf.readInt();
            irq_enabled = buf.readBoolean();
            init_state = buf.readInt();
        }
    }

    public void mapperInternalStateSave(ByteBuffer buf) {
        super.mapperInternalStateSave(buf);

        // Version:
        buf.putByte((short) 1);

        // State:
        buf.putInt(irq_counter);
        buf.putBoolean(irq_enabled);
        buf.putInt(init_state);

    }

    public void write(int address, short value) {

        int reg_num = (address & 0x7FFF) >> 13;

        if (address < 0x8000) {
            super.write(address, value);
        } else {
            if ((value & 0x80) != 0) {
                bits = 0;
                write_count = 0;
                if (reg_num == 0) {
                    regs[reg_num] |= 0x0C;
                }
            } else {
                bits |= (value & 1) << write_count++;
                if (write_count == 5) {
                    regs[reg_num] = bits & 0x1F;
                    bits = write_count = 0;
                }
            }

            if ((regs[0] & 0x02) != 0) {
                if ((regs[0] & 0x01) != 0) {
                    nes.getPpu().setMirroring(ROM.HORIZONTAL_MIRRORING);
                } else {
                    nes.getPpu().setMirroring(ROM.VERTICAL_MIRRORING);
                }
            } else {
                if ((regs[0] & 0x01) != 0) {
                    nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING2);
                } else {
                    nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING);
                }
            }

            switch (init_state) {
                case 0:
                case 1:
                     {
                        init_state++;
                    }
                    break;
                case 2:
                     {
                        if ((regs[1] & 0x08) != 0) {
                            if ((regs[0] & 0x08) != 0) {
                                if ((regs[0] & 0x04) != 0) {
                                    load8kRomBank((regs[3] & 0x07) * 2 + 16, 0x8000);
                                    load8kRomBank((regs[3] & 0x07) * 2 + 17, 0xA000);
                                    load8kRomBank(30, 0xC000);
                                    load8kRomBank(31, 0xE000);
                                } else {
                                    load8kRomBank(16, 0x8000);
                                    load8kRomBank(17, 0xA000);
                                    load8kRomBank((regs[3] & 0x07) * 2 + 16, 0xC000);
                                    load8kRomBank((regs[3] & 0x07) * 2 + 17, 0xE000);
                                }
                            } else {
                                load8kRomBank((regs[3] & 0x06) * 2 + 16, 0x8000);
                                load8kRomBank((regs[3] & 0x06) * 2 + 17, 0xA000);
                                load8kRomBank((regs[3] & 0x06) * 2 + 18, 0xC000);
                                load8kRomBank((regs[3] & 0x06) * 2 + 19, 0xE000);
                            }
                        } else {
                            load8kRomBank((regs[1] & 0x06) * 2 + 0, 0x8000);
                            load8kRomBank((regs[1] & 0x06) * 2 + 1, 0xA000);
                            load8kRomBank((regs[1] & 0x06) * 2 + 2, 0xC000);
                            load8kRomBank((regs[1] & 0x06) * 2 + 3, 0xE000);
                        }

                        if ((regs[1] & 0x10) != 0) {
                            irq_counter = 0;
                            irq_enabled = false;
                        } else {
                            irq_enabled = true;
                        }
                    }
                    break;
            }
        }
    }

    public int syncH(int scanline) {

        if (scanline == 0) {
            if (irq_enabled) {
                irq_counter += 29781;
            }
            if (((irq_counter | 0x21FFFFFF) & 0x3E000000) == 0x3E000000) {
                return 3;
            }
        }

        return 0;

    }

    public void loadROM(ROM rom) {

        if (!rom.isValid()) {
            //System.out.println("Invalid ROM! Unable to load.");
            return;
        }

        // Init:
        load8kRomBank(0, 0x8000);
        load8kRomBank(1, 0xA000);
        load8kRomBank(2, 0xC000);
        load8kRomBank(3, 0xE000);

        loadCHRROM();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);

    }

    public void reset() {

        regs[0] = 0x0C;
        regs[1] = 0x00;
        regs[2] = 0x00;
        regs[3] = 0x10;

        bits = 0;
        write_count = 0;

        irq_enabled = false;
        irq_counter = 0;
        init_state = 0;
    }
}