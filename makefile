
build: setup src/main.cpp src/rotor_control.cpp include/ProtocolRoTxCfg.h include/settings_params_legacy.h
	g++ -o build/rotor_control src/main.cpp -I. -I./include/

buildtest: setup src/step_function_test.cpp src/rotor_control.cpp include/ProtocolRoTxCfg.h include/settings_params_legacy.h
	g++ -o build/step_function_test src/step_function_test.cpp -I. -I./include/

setup:
	if ! [ -d "build" ]; then mkdir build; fi
	