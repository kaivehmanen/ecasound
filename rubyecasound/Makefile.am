AUTOMAKE_OPTIONS 		= foreign
EXTRA_DIST 			= README \
				  ecasound.rb \
				  control-interface.rb

# ----------------------------------------------------------------------
# defines
# ----------------------------------------------------------------------

if ECA_AM_RUBYECASOUND_INSTALL
rubyecasound_install_list1 = 	$(srcdir)/ecasound.rb \
				$(srcdir)/control-interface.rb
rubyecasound_uninstall_list1 =  $(DESTDIR)$(ECA_S_RUBY_SITEDIR)/ecasound.rb \
				$(DESTDIR)$(ECA_S_RUBY_SITEDIR)/control-interface.rb
endif

if ECA_AM_RUBYECASOUND_INSTALL
TESTS				= 
endif

# ----------------------------------------------------------------------
# hooks
# ----------------------------------------------------------------------

if ECA_AM_RUBYECASOUND_INSTALL
install-exec-hook: $(rubyecasound_install_list1) $(rubyecasound_install_list2)
	$(INSTALL) -d $(DESTDIR)$(ECA_S_RUBY_SITEDIR)
	$(INSTALL) $(rubyecasound_install_list1) $(rubyecasound_install_list2) $(DESTDIR)$(ECA_S_RUBY_SITEDIR)/
else
install-exec-hook:
endif

if ECA_AM_RUBYECASOUND_INSTALL
uninstall-local:
	rm -f $(rubyecasound_uninstall_list1) $(rubyecasound_uninstall_list2)
	rmdir --ignore-fail-on-non-empty $(DESTDIR)$(ECA_S_RUBY_SITEDIR)
else
uninstall-local:
endif
