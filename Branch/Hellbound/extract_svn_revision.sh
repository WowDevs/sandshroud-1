#!/bin/sh

# SVN Revision Extractor for Unix systems
# Written by Burlex, 2008/02/20

echo "// This file was automatically generated by the SVN revision extractor." > svn_revision.h
echo "// There is no need to modify it." >> svn_revision.h
echo "" >> svn_revision.h
echo "#ifndef SVN_REVISION_H" >> svn_revision.h
echo "#define SVN_REVISION_H" >> svn_revision.h
echo "" >> svn_revision.h
echo "#define BUILD_REVISION " `svn info | grep Revision | cut -d' ' -f2` >> svn_revision.h
echo "#define BUILD_DATE __DATE__" >> svn_revision.h
echo "#define BUILD_TIME __TIME__" >> svn_revision.h
echo "#define BUILD_HOST \""`hostname`"\"" >> svn_revision.h
echo "#define BUILD_USER \""`whoami`"\"" >> svn_revision.h
echo "" >> svn_revision.h
echo "#endif         // SVN_REVISION_H" >> svn_revision.h
echo "" >> svn_revision.h

mv svn_revision.h src/hearthstone-shared

echo "1"

