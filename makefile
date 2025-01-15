build: src/rotor_controller.cpp src/ProtocolRoTxCfg.h src/settings_params_legacy.h
	g++ -o build/rotor_control src/rotor_controller.cpp -I.