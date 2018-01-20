ID = -I/usr/local/include
LD = -L/usr/local/lib
LIBS = -lboost_serialization -lboost_system -lboost_program_options
SOURCEDIR = .
OBJ = KirbyAgent.o KnowledgeBase.o Rule.o Element.o Dictionary.o IndexFactory.o Prefices.o LogBox.o Parameters.o MT19937.o
OBJS = $(addprefix ${SOURCEDIR}/, $(OBJ))
HD = Distance.hpp
HDS = $(addprefix ${SOURCEDIR}/, $(HD))
OPT = --std=c++14 -O2

ki: ${OBJS}
	${CXX} ${OPT} ${SOURCEDIR}/KILM_main.cpp ${OBJS} ${LD} ${LIBS} -o ${SOURCEDIR}/kilm.exe

$(SOURCEDIR)/%.o: $(SOURCEDIR)/%.cpp
	@[ -d $(SOURCEDIR/) ]
	${CXX} ${OPT} ${ID} ${LD} ${LIBS} -o $@ -c $<

boost:
	${CXX} ${ID} ${SOURCEDIR}/boost_version.cpp -o b_ver.exe

$(SOURCEDIR)/KILM_main.cpp: ${OBJS} ${HDS} KILM_main.h
$(SOURCEDIR)/KirbyAgent.o: $(SOURCEDIR)/KnowledgeBase.o $(SOURCEDIR)/LogBox.o $(SOURCEDIR)/KirbyAgent.h
$(SOURCEDIR)/KnowledgeBase.o: ${HDS} $(SOURCEDIR)/Rule.o $(SOURCEDIR)/IndexFactory.o $(SOURCEDIR)/Prefices.o $(SOURCEDIR)/LogBox.o $(SOURCEDIR)/KnowledgeBase.h
Rule.o: $(SOURCEDIR)/Element.o $(SOURCEDIR)/Dictionary.o $(SOURCEDIR)/IndexFactory.o $(SOURCEDIR)/Prefices.o $(SOURCEDIR)/Rule.h
Element.o: $(SOURCEDIR)/Dictionary.o $(SOURCEDIR)/IndexFactory.o $(SOURCEDIR)/Prefices.o $(SOURCEDIR)/Element.h
Dictionary.o: $(SOURCEDIR)/Dictionary.h
IndexFactory.o: $(SOURCEDIR)/IndexFactory.h
Prefices.o: $(SOURCEDIR)/Prefices.h
LogBox.o: $(SOURCEDIR)/LogBox.h
MT19937.o: $(SOURCEDIR)/MT19937.h
Parameters.o: $(SOURCEDIR)/KnowledgeBase.o $(SOURCEDIR)/Parameters.h

clean:
	rm -f ${SOURCEDIR}/*.o ${SOURCEDIR}/*.dump ${SOURCEDIR}/*.exe ${SOURCEDIR}/*.log

