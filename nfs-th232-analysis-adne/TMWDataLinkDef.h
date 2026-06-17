#ifdef __CLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

// Classe principale
#pragma link C++ class TMWData+;

// Conteneurs STL utilisés dans la classe
#pragma link C++ class std::vector<UShort_t>+;
#pragma link C++ class std::vector<Float_t>+;
#pragma link C++ class std::vector<Int_t>+;
#pragma link C++ class std::vector<Short_t >+;

#endif
