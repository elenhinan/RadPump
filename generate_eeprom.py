Import("env")

# Custom HEX from ELF
env.AddPostAction(
    "$BUILD_DIR/firmware.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY",
        "--change-section-lma", ".eeprom=0",
        "--no-change-warning",
        "--set-section-flags=.eeprom='alloc,load'",
        "-j", ".eeprom",
        "-O", "ihex",
        "$BUILD_DIR/firmware.elf", "$BUILD_DIR/eeprom.hex"
    ]), "Creating $BUILD_DIR/eeprom.hex")
)