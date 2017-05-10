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

public class Mapper078 extends MapperDefault {

    public void init(NES nes) {

        super.init(nes);

    }

    public void write(int address, short value) {

        int prg_bank = value & 0x0F;
        int chr_bank = (value & 0xF0) >> 4;

        if (address < 0x8000) {
            super.write(address, value);
        } else {

            loadRomBank(prg_bank, 0x8000);
            load8kVromBank(chr_bank, 0x0000);

            if ((address & 0xFE00) != 0xFE00) {
                if ((value & 0x08) != 0) {
                    nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING2);
                } else {
                    nes.getPpu().setMirroring(ROM.SINGLESCREEN_MIRRORING);
                }
            }
        }
    }

    public void loadROM(ROM rom) {

        if (!rom.isValid()) {
            //System.out.println("Invalid ROM! Unable to load.");
            return;
        }

        int num_16k_banks = rom.getRomBankCount() * 4;

        // Init:
        loadRomBank(0, 0x8000);
        loadRomBank(num_16k_banks - 1, 0xC000);

        loadCHRROM();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);

    }
}