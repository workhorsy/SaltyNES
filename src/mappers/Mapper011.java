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

public class Mapper011 extends MapperDefault {

    public void init(NES nes) {

        super.init(nes);

    }

    public void write(int address, short value) {

        if (address < 0x8000) {

            // Let the base mapper take care of it.
            super.write(address, value);

        } else {

            // Swap in the given PRG-ROM bank:
            int prgbank1 = ((value & 0xF) * 2) % nes.getRom().getRomBankCount();
            int prgbank2 = ((value & 0xF) * 2 + 1) % nes.getRom().getRomBankCount();

            loadRomBank(prgbank1, 0x8000);
            loadRomBank(prgbank2, 0xC000);


            if (rom.getVromBankCount() > 0) {
                // Swap in the given VROM bank at 0x0000:
                int bank = ((value >> 4) * 2) % (nes.getRom().getVromBankCount());
                loadVromBank(bank, 0x0000);
                loadVromBank(bank + 1, 0x1000);
            }

        }

    }
}