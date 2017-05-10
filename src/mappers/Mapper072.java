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

public class Mapper072 extends MapperDefault {

    public void init(NES nes) {
        super.init(nes);
    }

    public void write(int address, short value) {

        if (address < 0x8000) {
            super.write(address, value);
        } else {
            int bank = value & 0x0f;
            int num_banks = rom.getRomBankCount();

            if ((value & 0x80) != 0) {
                loadRomBank(bank * 2, 0x8000);
                loadRomBank(num_banks - 1, 0xC000);
            }
            if ((value & 0x40) != 0) {
                load8kVromBank(bank * 8, 0x0000);
            }
        }
    }

    public void loadROM(ROM rom) {

        if (!rom.isValid()) {
            System.out.println("048: Invalid ROM! Unable to load.");
            return;
        }

        // Get number of 8K banks:
        int num_banks = rom.getRomBankCount() * 2;

        // Load PRG-ROM:
        loadRomBank(1, 0x8000);
        loadRomBank(num_banks - 1, 0xC000);

        // Load CHR-ROM:
        loadCHRROM();

        // Load Battery RAM (if present):
        // loadBatteryRam();

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);
    }
}