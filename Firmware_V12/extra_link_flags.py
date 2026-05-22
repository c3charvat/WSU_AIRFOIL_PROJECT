"""
extra_link_flags.py - PlatformIO extra script to add FPU machine flags
to the linker invocation.

The ststm32 platform builder script propagates -mthumb and -mcpu to LINKFLAGS
but does NOT automatically propagate -mfpu / -mfloat-abi from build_flags.
Without these, arm-none-eabi-g++ selects soft-float runtime libraries and the
linker rejects hard-float object files from the STM32 HAL.
"""

Import("env")

env.Append(
    LINKFLAGS=[
        "-mfpu=fpv4-sp-d16",
        "-mfloat-abi=hard",
    ]
)
