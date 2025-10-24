// Echo.Kern Dis VM Implementation in Go
//
// Dis virtual machine structures and execution engine.
// Based on Plan 9's Limbo/Dis architecture adapted for Echo.Kern.

package disvm

import (
	"fmt"
)

// OpCode represents Dis VM instruction opcodes
type OpCode uint8

const (
	// Memory operations
	OpLoad OpCode = iota
	OpStore
	OpAlloc

	// Arithmetic (using prime powers only!)
	OpMulPrime  // Multiply by prime
	OpExpPrime  // Exponentiate prime
	OpFactorize // Prime factorization

	// Control flow
	OpCall
	OpRet
	OpJump
	OpBranch

	// Membrane operations
	OpMembraneCreate
	OpMembraneEvolve
	OpMembraneSend

	// Hypergraph operations
	OpNodeCreate
	OpEdgeCreate
	OpGraphTraverse
)

// Instruction represents a Dis VM instruction
type Instruction struct {
	Op    OpCode
	Dst   uint32 // Destination register/address
	Src1  uint32 // Source 1
	Src2  uint32 // Source 2
	Prime uint64 // Prime number (for prime operations)
	Exp   uint32 // Exponent (for power operations)
}

// DisVM represents the Dis virtual machine state
type DisVM struct {
	// Registers (prime-indexed)
	Regs map[uint64]uint64 // Prime-indexed registers

	// Memory (hypergraph-based)
	Memory map[uint64]interface{} // Prime-indexed memory

	// Program counter
	PC uint32

	// Program code
	Code []Instruction

	// Call stack
	CallStack []uint32

	// OEIS A000081 context depth
	Depth uint32

	// Associated membrane
	MembraneID uint64
}

// NewDisVM creates a new Dis VM instance
func NewDisVM(depth uint32, membraneID uint64) *DisVM {
	return &DisVM{
		Regs:       make(map[uint64]uint64),
		Memory:     make(map[uint64]interface{}),
		PC:         0,
		CallStack:  make([]uint32, 0, 256),
		Depth:      depth,
		MembraneID: membraneID,
	}
}

// Execute executes one instruction
func (vm *DisVM) Execute(instr Instruction) error {
	switch instr.Op {
	case OpLoad:
		// Load from prime-indexed memory
		if val, ok := vm.Memory[uint64(instr.Src1)]; ok {
			vm.Regs[uint64(instr.Dst)] = val.(uint64)
		}

	case OpStore:
		// Store to prime-indexed memory
		val := vm.Regs[uint64(instr.Src1)]
		vm.Memory[uint64(instr.Dst)] = val

	case OpMulPrime:
		// Multiply by prime (NO ADDITION!)
		src := vm.Regs[uint64(instr.Src1)]
		vm.Regs[uint64(instr.Dst)] = src * instr.Prime

	case OpExpPrime:
		// Exponentiate prime: prime^exp
		result := uint64(1)
		for i := uint32(0); i < instr.Exp; i++ {
			result *= instr.Prime
		}
		vm.Regs[uint64(instr.Dst)] = result

	case OpFactorize:
		// Prime factorization
		val := vm.Regs[uint64(instr.Src1)]
		factors := primeFactor(val)
		// Store factorization result (simplified)
		vm.Memory[uint64(instr.Dst)] = factors

	case OpCall:
		// Call subroutine
		vm.CallStack = append(vm.CallStack, vm.PC)
		vm.PC = instr.Dst
		return nil

	case OpRet:
		// Return from subroutine
		if len(vm.CallStack) > 0 {
			vm.PC = vm.CallStack[len(vm.CallStack)-1]
			vm.CallStack = vm.CallStack[:len(vm.CallStack)-1]
		}

	case OpJump:
		// Unconditional jump
		vm.PC = instr.Dst
		return nil

	case OpBranch:
		// Conditional branch
		if vm.Regs[uint64(instr.Src1)] != 0 {
			vm.PC = instr.Dst
			return nil
		}

	case OpMembraneCreate:
		// Create new membrane
		fmt.Printf("Creating membrane at prime %d\n", instr.Prime)

	case OpMembraneEvolve:
		// Evolve membrane
		fmt.Printf("Evolving membrane %d\n", vm.MembraneID)

	case OpMembraneSend:
		// Send to membrane
		fmt.Printf("Sending to membrane %d\n", instr.Dst)

	default:
		return fmt.Errorf("unknown opcode: %d", instr.Op)
	}

	vm.PC++
	return nil
}

// Run executes the program until completion
func (vm *DisVM) Run() error {
	for vm.PC < uint32(len(vm.Code)) {
		instr := vm.Code[vm.PC]
		if err := vm.Execute(instr); err != nil {
			return err
		}
	}
	return nil
}

// primeFactor performs simple prime factorization
func primeFactor(n uint64) []uint64 {
	factors := make([]uint64, 0)
	// Simple trial division
	for i := uint64(2); i*i <= n; i++ {
		for n%i == 0 {
			factors = append(factors, i)
			n /= i
		}
	}
	if n > 1 {
		factors = append(factors, n)
	}
	return factors
}

// LoadProgram loads a program into the VM
func (vm *DisVM) LoadProgram(code []Instruction) {
	vm.Code = code
	vm.PC = 0
}

// OEIS A000081 values for concurrency
var OEIS_A000081 = []uint32{
	0, 1, 1, 2, 4, 9, 20, 48, 115, 286, 719, 1842, 4766, 12486,
}

// GetConcurrencyLevel returns OEIS A000081 value for depth
func GetConcurrencyLevel(depth uint32) uint32 {
	if depth < uint32(len(OEIS_A000081)) {
		return OEIS_A000081[depth]
	}
	return 0
}
