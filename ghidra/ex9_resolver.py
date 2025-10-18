# made by Edoardo Mantovani, 2025
# find automatically the "itb" register for the mt79xx firmware in ghidra and add a comment related to the instruction pointed to the itb
# this is useful for the mt7902/mt7921/mt7922

import sys

from ghidra.program.model.lang import Register
from ghidra.program.model.listing import CodeUnit

if __name__ == "__main__":
	results = []
	program = getCurrentProgram()
	listing = program.getListing()

	language = program.getLanguage()
	# this will contains the final value of the 'itb' register
	itb_final_value = None
	# the "itb" register is already defined in the nds32 ghidra's plugin
	itb_register = language.getRegister("itb")
	if itb_register is None:
		print("Register itb not found")
		sys.exit(0)

	instruction_iterator = listing.getInstructions(True)
	# iteration time !
	for single_instruction in instruction_iterator:
		input_objects = single_instruction.getInputObjects()
		is_using_itb = False

		for obj in input_objects:
			if isinstance(obj, Register):
				if obj.getName() == "itb" or itb_register.contains(obj) or obj.contains(itb_register):
					is_using_itb = True
					break

		if not is_using_itb:
			output_objects = single_instruction.getResultObjects()
			for obj in output_objects:
				if isinstance(obj, Register):
					if obj.getName() == "itb" or itb_register.contains(obj) or obj.contains(itb_register):
						is_using_itb = True
						break

		# store the result of the itb register in the array
		if is_using_itb:
			itb_reg_address      = single_instruction.getAddress()
			itb_reg_mnemonic     = single_instruction.getMnemonicString()
			itb_full_instruction = str(single_instruction)
			# usually the disassembly for setting the itb register is:
			#       00913862 46 00 09 68     sethi      a0,0x968
			#	00913866 58 00 0d e8     ori        a0,a0,0xde8
			#	0091386a 42 0e 00 21     mtusr      a0,itb
			# so we need to save the register of mtusr (a0), go back of 8 bytes and compare the "sethi" and "ori" second parameter

			if "mtusr" in single_instruction.getMnemonicString():
				itb_start_address = single_instruction.getAddress().subtract(8)
				# the 'first_instruction' should be always a 'sethi'
				first_instruction = getInstructionAt(itb_start_address)
				sethi_operand     = first_instruction.getDefaultOperandRepresentation(1)
				itb_final_value   = int(sethi_operand, 16) << 12
				# now process the 'ori' instruction
				ori_instr	  = listing.getInstructionAfter(first_instruction.getAddress())
				itb_final_value  |= int(ori_instr.getDefaultOperandRepresentation(2), 16)

		if itb_final_value is not None:
			itb_final_addr = toAddr(itb_final_value)
			for ex9_insn in listing.getInstructions(currentAddress, True):
				if ex9_insn.getMnemonicString().lower().startswith("ex9"):
					current_idx    		= int(ex9_insn.getDefaultOperandRepresentation(0), 0)
					current_entry  		= itb_final_addr.add(current_idx * 4)
					current_opcode 		= getInt(current_entry) & 0xffffffff
					instruction_iterator 	= listing.getInstructions(current_entry, True)

					# get only the first instruction
					for instruction in instruction_iterator:
						break

					# let's mantain this easy.
					listing.setComment(ex9_insn.getMinAddress(), CodeUnit.EOL_COMMENT, instruction.toString())
					print "ex9.it %s current index=%s" % (
						instruction.toString(), current_idx
					)
