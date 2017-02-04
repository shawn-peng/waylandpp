

$(OBJDIR)%.o: $(PROJDIR)%.cpp
	mkdir -p $(dir $@)
	$(CXX) -c $< $(CXXFLAGS) -o $@


