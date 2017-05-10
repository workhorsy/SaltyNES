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

public class Mapper087 extends MapperDefault {

    public void init(NES nes) {
        super.init(nes);
    }

    public void writelow(int address, short value) {

        if (address < 0x6000) {
            // Let the base mapper take care of it.
            super.writelow(address, value);
        } else if (address == 0x6000) {
            int chr_bank = (value & 0x02) >> 1;
            load8kVromBank(chr_bank * 8, 0x0000);
        }
    }

    public void loadROM(ROM rom) {

        if (!rom.isValid()) {
            System.out.println("Invalid ROM! Unable to load.");
            return;
        }

        // Get number of 8K banks:
        int num_8k_banks = rom.getRomBankCount() * 2;

        // Load PRG-ROM:
        load8kRomBank(0, 0x8000);
        load8kRomBank(1, 0xA000);
        load8kRomBank(2, 0xC000);
        load8kRomBank(3, 0xE000);

        // Load CHR-ROM:
        loadCHRROM();

        // Load Battery RAM (if present):

        // Do Reset-Interrupt:
        nes.getCpu().requestIrq(CPU.IRQ_RESET);
    }
}
