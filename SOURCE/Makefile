OUTPUT = a.out
LD = -L/usr/local/lib
LIBS = -lboost_serialization -lboost_thread -lboost_system -lboost_program_options
LIBS2 = ${LIBS} -ligraph
OBJ = KirbyAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o IndexFactory.o Prefices.o LogBox.o Parameters.o MT19937.o
ID = -I/usr/local/include/igraph
OBJ2 = ${OBJ} NetWorld.o LEILAParameters.o
OBJ3 = MSILMAgent.o KirbyAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o IndexFactory.o Prefices.o LogBox.o Parameters.o MT19937.o MSILMParameters.o
HD = Distance.hpp
#OPT = -O2
OPT = -g -O0

la: ${OBJ2}
	${CXX} ${OPT} ${ID} LEILA_main.cpp ${OBJ2} ${HD} ${LD} ${LIBS2} -o leila.exe

lag: ${OBJ2}
	${CXX} ${OPT} ${ID} LEILA_main.cpp ${OBJ2} ${HD} ${LD} ${LIBS2} -o leila.exe

ka: ${OBJ}
	${CXX} ${OPT} LEIKA_main.cpp ${OBJ} ${HD} ${LD} ${LIBS} -o leika.exe
	
ms: ${OBJ3}
	${CXX} ${OPT} MSILM_main.cpp ${OBJ3} ${HD} ${LD} ${LIBS} -o msilm.exe
	
cm: ${OBJ3}
	${CXX} ${OPT} CM_main.cpp ${OBJ3} ${HD} ${LD} ${LIBS} -o cmsilm.exe

.cpp.o:
	${CXX} ${OPT} -c $< ${ID} ${LD} ${LIBS2}

LEIKA_main.cpp: KirbyAgent.o LogBox.o Parameters.o MT19937.o
LEILA_main.cpp: KirbyAgent.o LogBox.o NetWorld.o LELAParameters.o MT19937.o
Kirby_main.o:Parameters.o Kirby_main.h
KirbyAgent.o: KnowledgeBase.o LogBox.o KirbyAgent.h
KnowledgeBase.o: ${HD} Rule.o IndexFactory.o Prefices.o LogBox.o KnowledgeBase.h
Rule.o: Element.o Dictionary.o IndexFactory.o Prefices.o Rule.h
Element.o:Dictionary.o IndexFactory.o Prefices.o Element.h
Dictionary.o:Dictionary.h
IndexFactory.o:IndexFactory.h
Prefices.o:Prefices.h
LogBox.o:LogBox.h
MT19937.o:MT19937.h
Parameters.o:Parameters.h
MSILMParameters.o:Parameters.o MSILMParameters.h
MSILMAgent.o:KirbyAgent.o MSILMAgent.h
MSILM_main.cpp: MSILMAgent.o Rule.o Element.o MT19937.o Dictionary.o LogBox.o MSILMParameters.o MSILM_main.h
CM_main.cpp: MSILMAgent.o Rule.o Element.o MT19937.o Dictionary.o LogBox.o MSILMParameters.o CM_main.h

clean:
	rm -f *.o *.dump *.exe
