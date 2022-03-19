#ifndef COPY_PROPAGATION_PASS_HPP
#define COPY_PROPAGATION_PASS_HPP

#include "FunctionPass.hpp"

/// For now do only local optimization (only optimize at basic block level).
/// The idea is to find loads and stores and register what value was loaded
/// or stored to the memory location, so subsequent loads from the same memory
/// location could be made dead by propagating the loaded or stored value to
/// the uses of the subsequent instructions.
/// Below is an IR snippet from the gcd.c, which can be found in the tests
/// folder. It is a single basic block.
///
///  ...
/// .loop_body0:
/// 	ld	$9<i32>, [$0<*i32>]
/// 	ld	$10<i32>, [$2<*i32>]
/// 	mod	$11<i32>, $9<i32>, $10<i32>
/// 	str	[$4<*i32>], $11<i32>
/// 	ld	$12<i32>, [$2<*i32>]
/// 	str	[$0<*i32>], $12<i32>
/// 	ld	$13<i32>, [$4<*i32>]
/// 	str	[$2<*i32>], $13<i32>
/// 	j	<loop_header0>
///   ...
///
/// Here the following can be seen after inspection
///   * The 3rd load (ld) instruction loading in a value from the same
///     location where the 2nd loaded it's value AND there are no store (str)
///     instructions between these two, which write to the memory location,
///     which from the value was loaded from. This means that $10 holds the same
///     value as $12 will. Therefore the 3rd load is redundant and $10 could
///     be used in places where $12 would have been used.
///
///   * The 4th load is loading from the memory location where the mod result
///     was stored. Since there were no other stores which would write to the
///     same memory location, therefore the memory location still holds the mod
///     result at this point. This means that, rather then loading it is from
///     memory, the mod result register, $11 can be used instead.
class CopyPropagationPass : public FunctionPass {
public:
  bool RunOnFunction(Function &F) override;
};

#endif // COPY_PROPAGATION_PASS_HPP
