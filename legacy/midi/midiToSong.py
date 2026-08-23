import mido

def midi_to_cpp(filename, array_name="melody", tempo_bpm=120):
    mid = mido.MidiFile(filename)
    ticks_per_beat = mid.ticks_per_beat
    ms_per_tick = (60000 / tempo_bpm) / ticks_per_beat

    notes = []
    abs_ticks = 0
    current_note = None
    note_start_tick = 0
    last_event_end_tick = 0

    for msg in mid:
        abs_ticks += msg.time

        # NOTE ON
        if msg.type == "note_on" and msg.velocity > 0:
            # rest before this note
            rest_ticks = abs_ticks - last_event_end_tick
            if rest_ticks > 0 and last_event_end_tick != 0:
                notes.append((0.0, int(rest_ticks * ms_per_tick)))
            current_note = msg.note
            note_start_tick = abs_ticks

        # NOTE OFF
        elif msg.type == "note_off" or (msg.type == "note_on" and msg.velocity == 0):
            if current_note is not None:
                dur_ticks = abs_ticks - note_start_tick
                freq = 440.0 * 2 ** ((current_note - 69) / 12)
                notes.append((freq, int(dur_ticks * ms_per_tick)))
                last_event_end_tick = abs_ticks
                current_note = None

    # C++ array
    cpp = f"Note {array_name}[{len(notes)}] = {{\n  "
    cpp += ", ".join([f"{{{freq:.2f}, {dur}}}" for freq, dur in notes])
    cpp += "\n};"
    return cpp

if __name__ == "__main__":
    import sys
    filename = sys.argv[1]
    array_name = sys.argv[2] if len(sys.argv) > 2 else "melody"
    tempo = float(sys.argv[3]) if len(sys.argv) > 3 else 120
    print(midi_to_cpp(filename, array_name, tempo))
