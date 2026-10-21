// After a remote update, the ESP32 keeps the previous version until the new one
// says it works. Returning true here means "we'll confirm it ourselves" - the
// program calls markFirmwareGood() once it has loaded stats or been set up.
// If the new version crashes or restarts before that, the board goes back to
// the previous version automatically.
extern "C" bool verifyRollbackLater() {
  return true;
}
