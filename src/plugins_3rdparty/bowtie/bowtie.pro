include (bowtie.pri)

DEPENDPATH += . \
              src/bowtie \
              src/bowtie_tests \
              src/bowtie/SeqAn-1.1 \
              src/bowtie/SeqAn-1.1/seqan \
              src/bowtie/SeqAn-1.1/seqan/basic \
              src/bowtie/SeqAn-1.1/seqan/file \
              src/bowtie/SeqAn-1.1/seqan/find \
              src/bowtie/SeqAn-1.1/seqan/index \
              src/bowtie/SeqAn-1.1/seqan/sequence
INCLUDEPATH += . \
              src/bowtie \
              src/bowtie/SeqAn-1.1

win32{

INCLUDEPATH += src/bowtie/visualstudio
DEPENDPATH += src/bowtie/visualstudio

}

# Input

FORMS += src/ui/BowtieSettings.ui

HEADERS += src/BowtieAdapter.h \
           src/BowtieConstants.h \
           src/BowtieContext.h \
           src/BowtieIOAdapter.h \ 
           src/BowtiePlugin.h \ 
           src/BowtieReadsIOUtils.h \
           src/BowtieSettingsWidget.h \
           src/BowtieTask.h \
           src/BowtieWorker.h \
           src/bowtie_tests/bowtieTests.h \
           src/bowtie/aligner.h \
           src/bowtie/aligner_0mm.h \
           src/bowtie/aligner_1mm.h \
           src/bowtie/aligner_23mm.h \
           src/bowtie/aligner_metrics.h \
           src/bowtie/aligner_seed_mm.h \
           src/bowtie/alphabet.h \
           src/bowtie/annot.h \
           src/bowtie/assert_helpers.h \
           src/bowtie/auto_array.h \
           src/bowtie/binary_sa_search.h \
           src/bowtie/bitpack.h \
           src/bowtie/bitset.h \
           src/bowtie/blockwise_sa.h \
           src/bowtie/color.h \
           src/bowtie/color_dec.h \
           src/bowtie/diff_sample.h \
           src/bowtie/ebwt.h \
           src/bowtie/ebwt_search_backtrack.h \
           src/bowtie/ebwt_search_util.h \
           src/bowtie/edit.h \
           src/bowtie/endian_swap.h \
           src/bowtie/filebuf.h \
           src/bowtie/formats.h \
           src/bowtie/hit.h \
           src/bowtie/hit_set.h \
           src/bowtie/log.h \
           src/bowtie/mm.h \
           src/bowtie/multikey_qsort.h \
           src/bowtie/pat.h \
           src/bowtie/pool.h \
           src/bowtie/qual.h \
           src/bowtie/random_source.h \
           src/bowtie/range.h \
           src/bowtie/range_cache.h \
           src/bowtie/range_chaser.h \
           src/bowtie/range_source.h \
           src/bowtie/ref_aligner.h \
           src/bowtie/ref_read.h \
           src/bowtie/reference.h \
           src/bowtie/refmap.h \
           src/bowtie/row_chaser.h \
           src/bowtie/sam.h \
           src/bowtie/search_globals.h \
           src/bowtie/sequence_io.h \
           src/bowtie/shmem.h \
           src/bowtie/spinlock.h \
           src/bowtie/str_util.h \
           src/bowtie/threading.h \
           src/bowtie/timer.h \
           src/bowtie/tokenize.h \
           src/bowtie/word_io.h \
           src/bowtie/zbox.h \
#SeqAn
           src/bowtie/SeqAn-1.1/seqan/basic.h \
           src/bowtie/SeqAn-1.1/seqan/file.h \
           src/bowtie/SeqAn-1.1/seqan/find.h \
           src/bowtie/SeqAn-1.1/seqan/index.h \
           src/bowtie/SeqAn-1.1/seqan/platform.h \
           src/bowtie/SeqAn-1.1/seqan/sequence.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_aggregates.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_allocator_interface.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_allocator_multipool.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_allocator_simple.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_allocator_singlepool.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_allocator_to_std.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_alphabet_interface.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_alphabet_interface2.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_alphabet_simple.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_alphabet_simple_tabs.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_alphabet_trait_basic.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_compare.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_converter.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_counted_ptr.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_debug.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_definition.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_generated_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_holder.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_host.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_iterator.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_iterator_adapt_std.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_iterator_adaptor.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_iterator_base.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_iterator_position.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_iterator_simple.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_metaprogramming.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_operator.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_pointer.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_profile.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_proxy.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_tag.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_transport.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_type.h \
           src/bowtie/SeqAn-1.1/seqan/basic/basic_volatile_ptr.h \
           src/bowtie/SeqAn-1.1/seqan/file/chunk_collector.h \
           src/bowtie/SeqAn-1.1/seqan/file/cstream.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_array.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_base.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_format.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_format_cgviz.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_format_fasta.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_format_fasta_align.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_format_guess.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_format_raw.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/file/file_generated_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/file/meta.h \
           src/bowtie/SeqAn-1.1/seqan/file/stream.h \
           src/bowtie/SeqAn-1.1/seqan/file/stream_algorithms.h \
           src/bowtie/SeqAn-1.1/seqan/find/find_base.h \
           src/bowtie/SeqAn-1.1/seqan/find/find_generated_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/find/find_horspool.h \
           src/bowtie/SeqAn-1.1/seqan/find/find_multi.h \
           src/bowtie/SeqAn-1.1/seqan/find/find_multiple_shiftand.h \
           src/bowtie/SeqAn-1.1/seqan/find/find_score.h \
           src/bowtie/SeqAn-1.1/seqan/find/find_set_horspool.h \
           src/bowtie/SeqAn-1.1/seqan/index/index_base.h \
           src/bowtie/SeqAn-1.1/seqan/index/index_find.h \
           src/bowtie/SeqAn-1.1/seqan/index/index_generated_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/index/index_manual_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/index/index_qgram_find.h \
           src/bowtie/SeqAn-1.1/seqan/index/index_sa_lss.h \
           src/bowtie/SeqAn-1.1/seqan/index/shape_base.h \
           src/bowtie/SeqAn-1.1/seqan/index/shape_gapped.h \
           src/bowtie/SeqAn-1.1/seqan/index/shape_predefined.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/lexical.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/segment_base.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/segment_infix.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/segment_prefix.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/segment_suffix.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/sequence_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/sequence_generated_forwards.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/sequence_interface.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/sequence_multiple.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/sequence_shortcuts.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/std_string.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_alloc.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_array.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_base.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_cstyle.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_packed.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_pointer.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_stack.h \
           src/bowtie/SeqAn-1.1/seqan/sequence/string_value_expand.h
           
SOURCES += src/BowtieAdapter.cpp \
           src/BowtieContext.cpp \
           src/BowtieIOAdapter.cpp \
           src/BowtiePlugin.cpp \ 
           src/BowtieSettingsWidget.cpp \
           src/BowtieTask.cpp \
           src/BowtieWorker.cpp \
           src/bowtie_tests/bowtieTests.cpp \
           src/bowtie/alphabet.cpp \
           src/bowtie/annot.cpp \
           src/bowtie/blockwise_sa.cpp \
           src/bowtie/ccnt_lut.cpp \
           src/bowtie/chaincat.cpp \
           src/bowtie/color.cpp \
           src/bowtie/color_dec.cpp \
           src/bowtie/ebwt.cpp \
           src/bowtie/ebwt_build.cpp \
           src/bowtie/ebwt_search.cpp \
           src/bowtie/ebwt_search_util.cpp \
           src/bowtie/edit.cpp \
           src/bowtie/hit.cpp \
           src/bowtie/hit_set.cpp \
           src/bowtie/log.cpp \
           src/bowtie/pat.cpp \
           src/bowtie/qual.cpp \
           src/bowtie/ref_aligner.cpp \
           src/bowtie/ref_read.cpp \
           src/bowtie/refmap.cpp \
           src/bowtie/sam.cpp \
           src/bowtie/shmem.cpp