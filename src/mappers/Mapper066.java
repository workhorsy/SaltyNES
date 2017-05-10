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

public class Mapper066 extends MapperDefault {

    public void init(NES nes) {

        super.init(nes);

    }

    public void write(int address, short value) {

        if (address < 0x8000) {

            // Let the base mapper take care of it.
            super.write(address, value);

        } else {

            // Swap in the given PRG-ROM bank at 0x8000:
            load32kRomBank((value >> 4) & 3, 0x8000);

            // Swap in the given VROM bank at 0x0000:
            load8kVromBank((value & 3) * 2, 0x0000);

        }

    }
}