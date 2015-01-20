make_wave.exe < musicdata/track_data01.txt > musicdata/output01.txt
make_wave.exe < musicdata/track_data02.txt > musicdata/output02.txt
make_wave.exe < musicdata/track_data03.txt > musicdata/output03.txt
make_wave.exe < musicdata/track_data04.txt > musicdata/output04.txt
make_wave.exe < musicdata/track_data05.txt > musicdata/output05.txt
mixer.exe musicdata/output01.txt musicdata/output02.txt musicdata/output03.txt musicdata/output04.txt musicdata/output05.txt > musicdata/output.txt
wave_gen.exe -44100 musicdata/output.txt out.wav