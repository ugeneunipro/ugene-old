<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru_RU">
<context>
    <name>HMMBuildDialog</name>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="32"/>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="49"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="14"/>
        <source>HMM Build</source>
        <translation>Построение HMM профайла</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="22"/>
        <source>Multiple alignment file:</source>
        <translation>Файл множественного выравнивания:</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="39"/>
        <source>File to save HMM profile:</source>
        <translation>Файл HMM профайла:</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="58"/>
        <source>Expert options</source>
        <translation>Дополнительные опции</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="70"/>
        <source>Name can be any string of non-whitespace characters (e.g. one ”word”).</source>
        <translation>Удобное для человека имя профайла, не должно содержать пропусков.</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="73"/>
        <source>Name this HMM:</source>
        <translation>Имя HMM профайла:</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="84"/>
        <source>
             By default, the model is configured to find one or more nonoverlapping alignments to the complete model:
             multiple global alignments with respect to the model, and local with respect to the sequence
         </source>
        <translation>По умолчанию, профайл настраивается на поиск одного или более неперекрывающихся выравниваний к полной модели:
множественные выравнивания глобальные относительно профайла, локальные относительно последовательности</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="90"/>
        <source>Default (hmmls) behaviour:</source>
        <translation>Стандартное поведение (hmmls):</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="120"/>
        <source>
             Configure the model for finding multiple domains per sequence, where each domain can be a local (fragmentary) alignment.
             This is analogous to the old hmmfs program of HMMER 1.
         </source>
        <translation>Профайл настраивается на поиск множественных доменов, каждый из которых может быть локальным (фрагментарным) выравниванием.
Это аналог программы hmmfs пакета HMMER 1.</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="126"/>
        <source>Emulate hmmfs behaviour:</source>
        <translation>Эмуляция hmmfs:</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="153"/>
        <source>
             Configure the model for finding a single global alignment to a target sequence,
             analogous to the old hmms program of HMMER 1.
         </source>
        <translation>Профайл настраивается на поиск единственного глобального выравнивания в целевой последовательности.
Это аналог программы hmms пакета HMMER 1.</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="159"/>
        <source>Emulate hmms behaviour:</source>
        <translation>Эмуляция hmms:</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="186"/>
        <source>
             Configure the model for finding a single local alignment per target sequence.
             This is analogous to the standard Smith/Waterman algorithm or the hmmsw program of HMMER 1.
         </source>
        <translation>Профайл настраивается на поиск единственного локального выравнивания в целевой последовательности.
Это аналог стандартного алгоритма Смита-Ватермана или программы hmmsw пакета HMMER 1.</translation>
    </message>
    <message>
        <location filename="../src/u_build/ui/HMMBuildDialog.ui" line="192"/>
        <source>Emulate hmmsw behaviour:</source>
        <translation>Эмуляция hmmsw:</translation>
    </message>
</context>
<context>
    <name>HMMBuildWorker</name>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="89"/>
        <source>Fix the length of the random sequences to &lt;n&gt;, where &lt;n&gt; is a positive (and reasonably sized) integer. &lt;p&gt;The default is instead to generate sequences with a variety of different lengths, controlled by a Gaussian (normal) distribution.</source>
        <translation>Фиксированная длина случайных последовательностей, положительное число разумной величины.
По умолчанию, генерируются последовательности с нормальным (Гауссовым) распределением по длине.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="92"/>
        <source>Mean length of the synthetic sequences, positive real number. The default value is 325.</source>
        <translation>Средняя длина синтетических последовательностей.
Положительное число, 325 по умолчанию.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="94"/>
        <source>Number of synthetic sequences. If &lt;n&gt; is less than about 1000, the fit to the EVD may fail. &lt;p&gt;Higher numbers of &lt;n&gt; will give better determined EVD parameters. &lt;p&gt;The default is 5000; it was empirically chosen as a tradeoff between accuracy and computation time.</source>
        <translation>Количество синтетических последовательностей.
Значение по умолчанию (5000) было эмпирически определено как компромисс между точностью и временем вычисления.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="98"/>
        <source>Standard deviation of the synthetic sequence length. A positive number. &lt;p&gt;The default is 200. Note that the Gaussian is left-truncated so that no sequences have lengths &lt;= 0.</source>
        <translation>Стандартное отклонение длины синтетической последовательности.
Должно быть больше нуля, по умолчанию 200.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="101"/>
        <source>The random seed, where &lt;n&gt; is a positive integer. &lt;p&gt;The default is to use time() to generate a different seed for each run, &lt;p&gt;which means that two different runs of hmmcalibrate on the same HMM will give slightly different results. &lt;p&gt;You can use this option to generate reproducible results for different hmmcalibrate runs on the same HMM.</source>
        <translation>Случайная затравка положительным целым числом. 
По умолчанию используется текущее время в миллисекундах для каждого запуска,что приводит к небольшим и незначительным различиям при каждой калибровке одного и того же профайла.
Вы можете использовать эту опцию для получения воспроизводимых результатов калибровки.</translation>
    </message>
</context>
<context>
    <name>HMMCalibrateDialog</name>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="22"/>
        <source>HMM file: </source>
        <translation>Файл c HMM профайлом:</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="32"/>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="297"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="41"/>
        <source>Expert options</source>
        <translation>Дополнительные опции</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="53"/>
        <source>
             Fix the length of the random sequences to n
                 , where n is a positive (and reasonably sized) integer. 
The default is instead to generate sequences with a variety of different lengths, controlled by a Gaussian (normal) distribution.</source>
        <translation>Фиксированная длина случайных последовательностей, положительное число разумной величины.
По умолчанию, генерируются последовательности с нормальным (Гауссовым) распределением по длине.</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="59"/>
        <source>Fix the length of the random sequences to:</source>
        <translation>Фиксированная длина случайных последовательностей:</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="104"/>
        <source>Mean length of the synthetic sequences:</source>
        <translation>Средняя длина последовательностей:</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="146"/>
        <source>
             Number of synthetic sequences.
             If n is less than about 1000, the fit to the EVD may fail
             Higher numbers of n will give better determined EVD parameters. 
             The default is 5000; it was empirically chosen as a tradeoff between accuracy and computation time.</source>
        <translation>Количество синтетических последовательностей.
Значение по умолчанию (5000) было эмпирически определено как компромисс между точностью и временем вычисления.</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="153"/>
        <source>Number of synthetic sequences:</source>
        <translation>Количество последовательностей:</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="195"/>
        <source>
             Standard deviation of the synthetic sequence length.
             A positive number. The default is 200.
             Note that the Gaussian is left-truncated so that no sequences have lengths less or equal 0.
         </source>
        <translation>Стандартное отклонение длины синтетической последовательности.
Должно быть больше нуля, по умолчанию 200.</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="202"/>
        <source>Standard deviation:</source>
        <translation>Стандартное отклонение:</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="238"/>
        <source>
             The random seed, where n is a positive integer. 
             The default is to use time() to generate a different seed for each run, 
             which means that two different runs of hmmcalibrate on the same HMM will give slightly different results. 
             You can use this option to generate reproducible results for different hmmcalibrate runs on the same HMM.</source>
        <translation>Случайная затравка положительным целым числом. 
По умолчанию используется текущее время в миллисекундах для каждого запуска,что приводит к небольшим и незначительным различиям при каждой калибровке одного и того же профайла.
Вы можете использовать эту опцию для получения воспроизводимых результатов калибровки.</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="245"/>
        <source>Random seed:</source>
        <translation>Случайная затравка:</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="278"/>
        <source>Save calibrated profile to file</source>
        <translation>Сохранить откалиброванный HMM профайл</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="290"/>
        <source>Save calibrated profile to file:</source>
        <translation>Сохранить откалиброванный HMM профайл:</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="14"/>
        <source>HMM Calibrate</source>
        <translation>Калибровать HMM профайл</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/ui/HMMCalibrateDialog.ui" line="101"/>
        <source>Mean length of the synthetic sequences, positive real number. The default value is 325.</source>
        <translation>Средняя длина синтетических последовательностей.
Положительное число, 325 по умолчанию.</translation>
    </message>
</context>
<context>
    <name>HMMSearchDialog</name>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="23"/>
        <source>HMM Search</source>
        <translation>Поиск HMM сигналов в последовательности</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="37"/>
        <source>File with HMM profile:</source>
        <translation>Файл HMM профайла:</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="47"/>
        <source>...</source>
        <translation>...</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="62"/>
        <source>Expert options</source>
        <translation>Дополнительные опции</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="77"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="80"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="83"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="226"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="229"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="232"/>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="76"/>
        <source>E-value filtering can be used to exclude low-probability hits from result.</source>
        <translation>Отбрасывание результатов по Е-величине помогает исключить мало-вероятные сигналы.</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="86"/>
        <source>Filter results with E-value greater then:</source>
        <translation>Отбрасывать результаты с Е-величиной более:</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="119"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="122"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="125"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="135"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="138"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="141"/>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="77"/>
        <source>Score based filtering is an alternative to E-value filtering to exclude low-probability hits from result.</source>
        <translation>Помогает исключить мало-вероятные сигналы.</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="128"/>
        <source>Filter results with Score lower than:</source>
        <translation>Отбрасывать результаты с итоговой суммой меньше:</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="188"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="191"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="194"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="210"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="213"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="216"/>
        <source>Calculate the E-value scores as if we had seen a sequence database of &lt;n&gt; sequences.</source>
        <translation>Считать Е-величину как если бы имелась база из &lt;n&gt; последовательностей.</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="219"/>
        <source>Number of sequences in dababase:</source>
        <translation>Количество последовательностей в базе:</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="277"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="280"/>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="283"/>
        <source>Variants of algorithm</source>
        <translation>Версии алгоритма</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="241"/>
        <source>1E</source>
        <translation>1e</translation>
    </message>
    <message>
        <location filename="../src/u_search/ui/HMMSearchDialog.ui" line="257"/>
        <source>Algorithm</source>
        <translation>Алгоритм</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="75"/>
        <source>Calculate the E-value scores as if we had seen a sequence database of &amp;lt;n&amp;gt; sequences.</source>
        <translation>Считать Е-величину как если бы имелась база из &lt;n&gt; последовательностей.</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="68"/>
        <source>HMM Profile</source>
        <translation>Профайл HMM</translation>
    </message>
</context>
<context>
    <name>U2::GTest_uHMMERCalibrate</name>
    <message>
        <location filename="../src/u_tests/uhmmerTests.cpp" line="570"/>
        <source>uhmmer-calibrate-subtask</source>
        <translation>Калибровка HMM профайла</translation>
    </message>
</context>
<context>
    <name>U2::HMM2QDActor</name>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="65"/>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="150"/>
        <source>HMM2</source>
        <translation>HMM2</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="76"/>
        <source>QD HMM2 search</source>
        <translation>Искать сигналы HMM2</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="82"/>
        <source>Power of e-value must be less or equal to zero. Using default value: 1e-1</source>
        <translation>Степень e-value должна быть меньше или равна нулю. Исползутся значение по умолчанию: 1е-1</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="151"/>
        <source>Searches HMM signals in a sequence with one or more profile HMM2 and saves the results as annotations.</source>
        <translation>Поиск HMM сигналов в последовательности, используя один или несколько HMM2 профилей и сохранение результирующих аннотаций.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="157"/>
        <source>Profile HMM</source>
        <translation>Профиль HMM</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="158"/>
        <source>Semicolon-separated list of input HMM files.</source>
        <translation>Список входных HMM-профилей, разделенных точкой с запятой.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="161"/>
        <source>Min Length</source>
        <translation>Мин длина</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="162"/>
        <source>Minimum length of a result region.</source>
        <translation>Мин длина результирующего региона.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="165"/>
        <source>Max Length</source>
        <translation>Макс длина</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="166"/>
        <source>Maximum length of a result region.</source>
        <translation>Максимальная длина результируюшего региона.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="169"/>
        <source>Filter by High E-value</source>
        <translation>Фильтрация по высокому Е-значению</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="170"/>
        <source>Reports domains &amp;lt;= this E-value threshold in output.</source>
        <translation>Возвращать результаты &amp;lt;= данного E-value (математическое ожидание).</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="174"/>
        <source>Filter by Low Score</source>
        <translation>Фильтрация по низкому score</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="175"/>
        <source>Reports domains &amp;gt;= this score cutoff in output.</source>
        <translation>Возвращать результаты &amp;lt;= данного score (веса).</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="178"/>
        <source>Number of Sequences</source>
        <translation>Число последовательностей</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchQDActor.cpp" line="179"/>
        <source>Specifies number of significant sequences. It is used for domain E-value calculations.</source>
        <translation>Число значимых последовательностей. Используется для вычисления E-value.</translation>
    </message>
</context>
<context>
    <name>U2::HMMADVContext</name>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="257"/>
        <source>Find HMM signals with HMMER2...</source>
        <translation>Поиск с помощью скрытой марковской модели (HMMER2)...</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="274"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="274"/>
        <source>No sequences found</source>
        <translation>Не выбрана последовательность</translation>
    </message>
</context>
<context>
    <name>U2::HMMBuildDialogController</name>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="58"/>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="175"/>
        <source>Build</source>
        <translation>Построить</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="59"/>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="176"/>
        <source>Close</source>
        <translation>Close</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="80"/>
        <source>Select file with alignment</source>
        <translation>Выберете файл с выравниванием</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="92"/>
        <source>Select file with HMM profile</source>
        <translation>Выберете файл с HMM профайлом</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="112"/>
        <source>Incorrect alignment file!</source>
        <translation>Некорректный файл выравнивания!</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="117"/>
        <source>Incorrect HMM file!</source>
        <translation>Укажите файл с HMM профайлом!</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="133"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="142"/>
        <source>Starting build process</source>
        <translation>Начинается построение</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="145"/>
        <source>Hide</source>
        <translation>Скрыть</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="146"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="169"/>
        <source>HMM build finished with errors: %1</source>
        <translation>Построение HMM профайла завершилось с ошибкой: %1</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="171"/>
        <source>HMM build canceled</source>
        <translation>Построение HMM профайла отменено</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="173"/>
        <source>HMM build finished successfuly!</source>
        <translation>HMM профайл успешно построен!</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="185"/>
        <source>Progress: %1%</source>
        <translation>Прогресс: %1%</translation>
    </message>
</context>
<context>
    <name>U2::HMMBuildTask</name>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="325"/>
        <source>Build HMM profile &apos;%1&apos;</source>
        <translation>Построение HMM профала &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="342"/>
        <source>Multiple alignment is empty</source>
        <translation>Выравнивание не содержит данных</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="346"/>
        <source>Multiple alignment is of 0 length</source>
        <translation>Выравнивание длины 0</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="351"/>
        <source>Invalid alphabet! Only amino and nucleic alphabets are supported</source>
        <translation>Неподдерживаемый тип алфавита</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="359"/>
        <source>Error creating MSA structure</source>
        <translation>Недостаточно памяти для построения выравнивания</translation>
    </message>
</context>
<context>
    <name>U2::HMMBuildToFileTask</name>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="310"/>
        <source>none</source>
        <translation>нет</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="196"/>
        <source>Build HMM profile &apos;%1&apos; -&gt; &apos;%2&apos;</source>
        <translation>Построение HMM профайла &apos;%1&apos;-&gt;&apos;%2&apos;</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="209"/>
        <source>Error reading alignment file</source>
        <translation>Ошибка чтения файла</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="226"/>
        <source>Build HMM profile to &apos;%1&apos;</source>
        <translation>Построение HMM профайла &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="251"/>
        <source>Incorrect input file</source>
        <translation>Неправильный входной файл</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="256"/>
        <source>Alignment object not found!</source>
        <translation>Объект выравнивания не найден!</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="298"/>
        <source>Source alignment</source>
        <translation>Исходное выравнивание</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="299"/>
        <source>Profile name</source>
        <translation>Имя профайла</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="301"/>
        <source>Task was not finished</source>
        <translation>Задача была не завершена</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="306"/>
        <source>Profile file</source>
        <translation>Файл</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildDialogController.cpp" line="307"/>
        <source>Expert options</source>
        <translation>Дополнительные опции</translation>
    </message>
</context>
<context>
    <name>U2::HMMCalibrateDialogController</name>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="51"/>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="160"/>
        <source>Calibrate</source>
        <translation>Калибровать</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="52"/>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="161"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="65"/>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="74"/>
        <source>Select file with HMM model</source>
        <translation>Выбор HMM профайла для калибровки</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="95"/>
        <source>Incorrect HMM file!</source>
        <translation>Не указан файл c исходным HMM профайлом!</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="101"/>
        <source>Illegal fixed length value!</source>
        <translation>Некорректное значение фиксированной длины последовательностей!</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="118"/>
        <source>Invalid output file name</source>
        <translation>Не указан файл для сохранения результата</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="124"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="134"/>
        <source>Starting calibration process</source>
        <translation>Начинается калибровка</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="137"/>
        <source>Hide</source>
        <translation>Скрыть</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="138"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="154"/>
        <source>Calibration finished with errors: %1</source>
        <translation>Калибровка завершена с ошибкой: %1</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="156"/>
        <source>Calibration was cancelled</source>
        <translation>Калибровка отменена</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="158"/>
        <source>Calibration finished successfuly!</source>
        <translation>Калибровка успешно завершена!</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateDialogController.cpp" line="168"/>
        <source>Progress: %1%</source>
        <translation>Прогресс: %1%</translation>
    </message>
</context>
<context>
    <name>U2::HMMCalibrateParallelSubTask</name>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="137"/>
        <source>Parallel HMM calibration subtask</source>
        <translation>Подзадача параллельного калибрования</translation>
    </message>
</context>
<context>
    <name>U2::HMMCalibrateParallelTask</name>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="35"/>
        <source>HMM calibrate &apos;%1&apos;</source>
        <translation>HMM калибровка &apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>U2::HMMCalibrateTask</name>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="17"/>
        <source>HMM calibrate &apos;%1&apos;</source>
        <translation>HMM калибровка &apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>U2::HMMCalibrateToFileTask</name>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="161"/>
        <source>HMM calibrate &apos;%1&apos;</source>
        <translation>HMM калибровка &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="200"/>
        <source>Source profile</source>
        <translation>Исходный профайл</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="203"/>
        <source>Task was not finished</source>
        <translation>Задача была не завершена</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="208"/>
        <source>Result profile</source>
        <translation>Файл результата</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="209"/>
        <source>Expert options</source>
        <translation>Дополнительные опции</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="211"/>
        <source>Number of random sequences to sample</source>
        <translation>Кол-во случайных синтетических последовательностей</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="212"/>
        <source>Random number seed</source>
        <translation>Затравка</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="213"/>
        <source>Mean of length distribution</source>
        <translation>Средняя длина синтетических последовательностей</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="214"/>
        <source>Standard deviation of length distribution</source>
        <translation>Стандартное отклонение средней длины</translation>
    </message>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="216"/>
        <source>Calculated evidence (mu , lambda)</source>
        <translation>Вычисленная вероятность (мю, лямбда)</translation>
    </message>
</context>
<context>
    <name>U2::HMMCreateWPoolTask</name>
    <message>
        <location filename="../src/u_calibrate/HMMCalibrateTask.cpp" line="98"/>
        <source>Initialize parallel context</source>
        <translation>Инициализация контекста</translation>
    </message>
</context>
<context>
    <name>U2::HMMIO</name>
    <message>
        <location filename="../src/HMMIO.cpp" line="215"/>
        <location filename="../src/HMMIO.cpp" line="231"/>
        <location filename="../src/HMMIO.cpp" line="391"/>
        <location filename="../src/HMMIO.cpp" line="397"/>
        <location filename="../src/HMMIO.cpp" line="423"/>
        <location filename="../src/HMMIO.cpp" line="454"/>
        <location filename="../src/HMMIO.cpp" line="479"/>
        <source>Illegal line</source>
        <translation>Ошибка формата</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="219"/>
        <source>File format is not supported</source>
        <translation>Формат не поддерживается</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="277"/>
        <location filename="../src/HMMIO.cpp" line="282"/>
        <location filename="../src/HMMIO.cpp" line="289"/>
        <location filename="../src/HMMIO.cpp" line="294"/>
        <location filename="../src/HMMIO.cpp" line="301"/>
        <location filename="../src/HMMIO.cpp" line="306"/>
        <location filename="../src/HMMIO.cpp" line="313"/>
        <location filename="../src/HMMIO.cpp" line="319"/>
        <location filename="../src/HMMIO.cpp" line="328"/>
        <location filename="../src/HMMIO.cpp" line="333"/>
        <location filename="../src/HMMIO.cpp" line="345"/>
        <location filename="../src/HMMIO.cpp" line="354"/>
        <location filename="../src/HMMIO.cpp" line="359"/>
        <location filename="../src/HMMIO.cpp" line="403"/>
        <location filename="../src/HMMIO.cpp" line="408"/>
        <location filename="../src/HMMIO.cpp" line="412"/>
        <location filename="../src/HMMIO.cpp" line="429"/>
        <location filename="../src/HMMIO.cpp" line="433"/>
        <location filename="../src/HMMIO.cpp" line="438"/>
        <location filename="../src/HMMIO.cpp" line="445"/>
        <location filename="../src/HMMIO.cpp" line="460"/>
        <location filename="../src/HMMIO.cpp" line="469"/>
        <location filename="../src/HMMIO.cpp" line="485"/>
        <location filename="../src/HMMIO.cpp" line="493"/>
        <location filename="../src/HMMIO.cpp" line="499"/>
        <location filename="../src/HMMIO.cpp" line="504"/>
        <source>Invalid file structure near %1</source>
        <translation>Некорректная структура файла вблизи &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="339"/>
        <source>ALPH must precede NULE in HMM save files</source>
        <translation>ALPH must precede NULE in HMM save files</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="248"/>
        <location filename="../src/HMMIO.cpp" line="374"/>
        <location filename="../src/HMMIO.cpp" line="378"/>
        <source>Value is illegal: %1</source>
        <translation>Некорректное значение: &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="72"/>
        <source>Alphabet is not set</source>
        <translation>Не установлен алфавит</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="382"/>
        <source>Value is not set for &apos;%1&apos;</source>
        <translation>Не установлено значение для &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="518"/>
        <source>No &apos;//&apos; symbol found</source>
        <translation>Не найден маркер конца структуры: &apos;//&apos;</translation>
    </message>
    <message>
        <location filename="../src/HMMIO.cpp" line="636"/>
        <source>HMM models</source>
        <translation>Профайлы HMM</translation>
    </message>
</context>
<context>
    <name>U2::HMMMSAEditorContext</name>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="215"/>
        <source>Build HMMER2 profile</source>
        <translation>Построить профиль с помощью HMMER2</translation>
    </message>
</context>
<context>
    <name>U2::HMMReadTask</name>
    <message>
        <location filename="../src/HMMIO.cpp" line="651"/>
        <source>Read HMM profile &apos;%1&apos;.</source>
        <translation>Чтение HMM профайла &apos;%1&apos;.</translation>
    </message>
</context>
<context>
    <name>U2::HMMSearchDialogController</name>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="114"/>
        <source>Select file with HMM model</source>
        <translation>Выбор HMM профайла для калибровки</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="131"/>
        <source>HMM file not set!</source>
        <translation>Укажите файл с HMM профайлом!</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="148"/>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="154"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="154"/>
        <source>Cannot create an annotation object. Please check settings</source>
        <translation>Невозможно создать аннотацию. Проверьте настройки</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="165"/>
        <source>Starting search process</source>
        <translation>Начинается поиск</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="168"/>
        <source>Hide</source>
        <translation>Скрыть</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="169"/>
        <source>Cancel</source>
        <translation>Отмена</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="184"/>
        <source>HMM search finished with error: %1</source>
        <translation>Построение HMM профайла завершилось с ошибкой: %1</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="186"/>
        <source>HMM search finished successfuly!</source>
        <translation>HMM профайл успешно построен!</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="196"/>
        <source>Progress: %1%</source>
        <translation>Прогресс: %1%</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="77"/>
        <source>Cell BE optimized</source>
        <translation>Оптимизирован для Cell BE</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="61"/>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="188"/>
        <source>Search</source>
        <translation>Искать</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="62"/>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="189"/>
        <source>Close</source>
        <translation>Закрыть</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="80"/>
        <source>SSE optimized</source>
        <translation>Оптимизирован для SSE</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="82"/>
        <source>Conservative</source>
        <translation>Стандартный</translation>
    </message>
</context>
<context>
    <name>U2::HMMSearchTask</name>
    <message>
        <location filename="../src/u_search/HMMSearchTask.cpp" line="48"/>
        <source>HMM Search</source>
        <translation>Поиск HMM</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchTask.cpp" line="266"/>
        <source>Invalid HMM alphabet!</source>
        <translation>Неправильный алфавит HMM профайла!</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchTask.cpp" line="270"/>
        <source>Invalid sequence alphabet!</source>
        <translation>Неправильный алфавит последовательности!</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchTask.cpp" line="298"/>
        <source>Amino translation is not available for the sequence alphabet!</source>
        <translation>Не найдена таблица аминокислотной трансляции для выбранной последовательности!</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchTask.cpp" line="332"/>
        <source>Parallel HMM search</source>
        <translation>Параллельный поиск HMM сигналов</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchTask.cpp" line="40"/>
        <source>HMM search with &apos;%1&apos;</source>
        <translation>Поиск с HMM профайлом &apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>U2::HMMSearchToAnnotationsTask</name>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="215"/>
        <source>HMM search, file &apos;%1&apos;</source>
        <translation>HMM поиск, файл &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="221"/>
        <source>RAW alphabet is not supported!</source>
        <translation>Неправильный алфавит последовательности!</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="237"/>
        <source>Annotation object was removed</source>
        <translation>Объект аннотаций был удалён</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="264"/>
        <source>HMM profile used</source>
        <translation>Использованный HMM профайл</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="267"/>
        <source>Task was not finished</source>
        <translation>Задача была не завершена</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="272"/>
        <source>Result annotation table</source>
        <translation>Таблица аннотаций результата</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="273"/>
        <source>Result annotation group</source>
        <translation>Группа аннотаций результата</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="274"/>
        <source>Result annotation name</source>
        <translation>Имя аннотаций результата</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchDialogController.cpp" line="277"/>
        <source>Results count</source>
        <translation>Количество найденных регионов</translation>
    </message>
</context>
<context>
    <name>U2::HMMWriteTask</name>
    <message>
        <location filename="../src/HMMIO.cpp" line="671"/>
        <source>Write HMM profile &apos;%1&apos;</source>
        <translation>Запись HMM профайла &apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMBuildPrompter</name>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="182"/>
        <source>For each MSA from &lt;u&gt;%1&lt;/u&gt;,</source>
        <translation>Для каждого выравнивания из &lt;u&gt;%1&lt;/u&gt;,</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="186"/>
        <source> and calibrate</source>
        <translation> и калибровать</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="188"/>
        <source>default</source>
        <translation>настройки по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="188"/>
        <source>custom</source>
        <translation>выбранные настройки</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="190"/>
        <source>%1 build%2 HMM profile using &lt;u&gt;%3&lt;/u&gt; settings.</source>
        <translation>%1 построить%2 HMM профайл используя &lt;u&gt;%3&lt;/u&gt;.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMBuildWorker</name>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="296"/>
        <source>Built HMM profile</source>
        <translation>Построен профайл HMM</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="302"/>
        <source>Calibrated HMM profile</source>
        <translation>Откалиброван профайл HMM</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="122"/>
        <source>HMM Build</source>
        <translation>Построить HMM</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="65"/>
        <source>HMM profile</source>
        <translation>Профайл HMM</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="75"/>
        <source>HMM strategy</source>
        <translation>Стратегия HMM</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="75"/>
        <source>Specifies kind of alignments you want to allow.</source>
        <translation>Выбор типа искомого выравнивания.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="76"/>
        <source>Profile name</source>
        <translation>Имя профайла</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="76"/>
        <source>Descriptive name of the HMM profile.</source>
        <translation>Описательное имя целевого профайла HMM.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="83"/>
        <source>Calibrate profile</source>
        <translation>Калибровать</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="85"/>
        <source>Parallel calibration</source>
        <translation>Параллельная калибрация</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="88"/>
        <source>Fixed length of samples</source>
        <translation>Фиксированная длина сэмплов</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="91"/>
        <source>Mean length of samples</source>
        <translation>Средняя длина сэмплов</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="93"/>
        <source>Number of samples</source>
        <translation>Число сэмплов</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="97"/>
        <source>Standard deviation</source>
        <translation>Стандартное отклонение</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="100"/>
        <source>Random seed</source>
        <translation>Затравка</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="156"/>
        <source>Default</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="218"/>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="219"/>
        <source>Incorrect value for seed parameter</source>
        <translation>Некорректное значение для параметра затравки</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="237"/>
        <source>Schema name not specified. Using default value: &apos;%1&apos;</source>
        <translation>Не задано имя схемы. Использовано значение по умолчанию: &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="63"/>
        <source>Input MSA</source>
        <translation>Входное выравнивание</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="64"/>
        <source>Input multiple sequence alignment for building statistical model.</source>
        <translation>Множественное выравнивание последовательностей для построения статистической модели.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="65"/>
        <source>Produced HMM profile</source>
        <translation>Произведённый профайл HMM</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="83"/>
        <source>Enables/disables optional profile calibration.&lt;p&gt;An empirical HMM calibration costs time but it only has to be done once per model, and can greatly increase the sensitivity of a database search.</source>
        <translation>Включить/выключить калибрацию профайлов. &lt;p&gt;Однократная калибрация случайными синтетическими последовательностями занимает некоторое время, но может существенно улучшить чувствительность поиска.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="86"/>
        <source>Number of parallel threads that the calibration will run in.</source>
        <translation>Число потоков для параллельной калибрации.</translation>
    </message>
    <message>
        <location filename="../src/u_build/HMMBuildWorker.cpp" line="122"/>
        <source>Builds a HMM profile from a multiple sequence alignment.&lt;p&gt;The HMM profile is a statistical model which captures position-specific information about how conserved each column of the alignment is, and which residues are likely.</source>
        <translation>Строит профайлы HMM по входящим множественным выравниваниям.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMLib</name>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="70"/>
        <source>HMMER2 Tools</source>
        <translation>Программы HMMER2</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="62"/>
        <source>HMM Profile</source>
        <translation>Профайл HMM</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="132"/>
        <location filename="../src/HMMIOWorker.cpp" line="149"/>
        <source>HMM profile</source>
        <translation>Профайл HMM</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="133"/>
        <source>Location</source>
        <translation>Адрес</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="156"/>
        <source>Read HMM Profile</source>
        <translation>Читать HMM профайл</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="143"/>
        <source>Write HMM Profile</source>
        <translation>Сохранить HMM профайл</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="132"/>
        <source>Input HMM profile</source>
        <translation>Входной профайл HMM</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="133"/>
        <source>Location hint for the target file.</source>
        <translation>Адрес файла для записи.</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="143"/>
        <source>Saves all input HMM profiles to specified location.</source>
        <translation>Сохраняет все входящие профайлы HMM в указанный файл.</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="156"/>
        <source>Reads HMM profiles from file(s). The files can be local or Internet URLs.</source>
        <translation>Считывает профайлы HMM из одного или нескольких файлов. Файлы могут быть локальными или адресами Интернет.</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="149"/>
        <source>Loaded HMM profile</source>
        <translation>Загруженный профайл HMM</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMReadPrompter</name>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="168"/>
        <source>Read HMM profile(s) from %1.</source>
        <translation>Загрузить HMM профайл(ы) из %1.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMReader</name>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="235"/>
        <source>Loaded HMM profile from %1</source>
        <translation>Загружен HMM профайл из %1</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMSearchPrompter</name>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="127"/>
        <source>For each sequence from &lt;u&gt;%1&lt;/u&gt;,</source>
        <translation>Для каждой последовательности из &lt;u&gt;%1&lt;/u&gt;,</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="131"/>
        <source>Use &lt;u&gt;default&lt;/u&gt; settings.</source>
        <translation>Использовать настройки &lt;u&gt;по умолчанию&lt;/u&gt;.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="131"/>
        <source>Use &lt;u&gt;custom&lt;/u&gt; settings.</source>
        <translation>Использовать &lt;u&gt;указанные&lt;/u&gt; настройки.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="133"/>
        <source>%1 HMM signals%2. %3&lt;br&gt;Output the list of found regions annotated as &lt;u&gt;%4&lt;/u&gt;.</source>
        <translation>%1 HMM сигналы%2. %3&lt;br&gt;Выдать список найденных регионов аннотированных как &lt;u&gt;%4&lt;/u&gt;.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="128"/>
        <source>using all profiles provided by &lt;u&gt;%1&lt;/u&gt;,</source>
        <translation>со всеми профайлами из &lt;u&gt;%1&lt;/u&gt;,</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMSearchWorker</name>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="215"/>
        <source>Bad sequence supplied to input: %1</source>
        <translation>Некорректная последовательность: %1</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="238"/>
        <source>Found %1 HMM signals</source>
        <translation>Найдено %1 сигналов HMM</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="56"/>
        <source>HMM profile</source>
        <translation>HMM профайл</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="57"/>
        <source>Input sequence</source>
        <translation>Входная последовательность</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="59"/>
        <source>HMM annotations</source>
        <translation>Аннотации</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="74"/>
        <source>Result annotation</source>
        <translation>Имя аннотации</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="75"/>
        <source>Number of seqs</source>
        <translation>Число последовательностей</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="76"/>
        <source>Filter by high E-value</source>
        <translation>Фильтр на большие Е</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="77"/>
        <source>Filter by low score</source>
        <translation>Фильтр на малые веса</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="85"/>
        <source>HMM Search</source>
        <translation>Поиск HMM</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="158"/>
        <source>Power of e-value must be less or equal to zero. Using default value: 1e-1</source>
        <translation>Степень e-value должна быть меньше или равна нулю. Исползутся значение по умолчанию: 1е-1</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="167"/>
        <source>Value for attribute name is empty, default name used</source>
        <translation>Имя не указано, использовано значение по умолчанию</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="211"/>
        <source>Find HMM signals in %1</source>
        <translation>Поиск HMM сигналов в последовательности &apos;%1&apos;</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="56"/>
        <source>HMM profile(s) to search with.</source>
        <translation>HMM профайлы для поиска.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="58"/>
        <source>An input sequence (nucleotide or protein) to search in.</source>
        <translation>Входная последовательность для поиска (протеиновая или нуклеотидная).</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="60"/>
        <source>Annotations marking found similar sequence regions.</source>
        <translation>Аннотации размечающие найденные регионы совпадения с HMM.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="74"/>
        <source>A name of the result annotations.</source>
        <translation>Имя аннотации для разметки результатов.</translation>
    </message>
    <message>
        <location filename="../src/u_search/HMMSearchWorker.cpp" line="86"/>
        <source>Searches each input sequence for significantly similar sequence matches to all specified HMM profiles. In case several profiles were supplied, searches with all profiles one by one and outputs united set of annotations for each sequence.</source>
        <translation>Ищет значимые совпадения с указанными HMM профайлами в каждой входной последовательности и выдаёт набор аннотированных регионов. Профайлов может быть несколько, поиск отработает для каждого профайла и выдаст общий набор аннотаций для каждой последовательности.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMWritePrompter</name>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="174"/>
        <source>unset</source>
        <translation>не указан</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="178"/>
        <source>Save HMM profile(s) from &lt;u&gt;%1&lt;/u&gt; to &lt;u&gt;%2&lt;/u&gt;.</source>
        <translation>Сохранить HMM профайл(ы) из &lt;u&gt;%1&lt;/u&gt; в &lt;u&gt;%2&lt;/u&gt;.</translation>
    </message>
</context>
<context>
    <name>U2::LocalWorkflow::HMMWriter</name>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="259"/>
        <source>Empty HMM passed for writing to %1</source>
        <translation>Пустой HMM профайл для записи в %1</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="259"/>
        <source>Unspecified URL for writing HMM</source>
        <translation>Некорректный путь для записи HMM</translation>
    </message>
    <message>
        <location filename="../src/HMMIOWorker.cpp" line="270"/>
        <source>Writing HMM profile to %1</source>
        <translation>Запись HMM профайла &apos;%1&apos;</translation>
    </message>
</context>
<context>
    <name>U2::UHMMBuild</name>
    <message>
        <location filename="../src/u_build/uhmmbuild.cpp" line="184"/>
        <source>bogus configuration choice</source>
        <translation>некорректный выбор конфигурации</translation>
    </message>
</context>
<context>
    <name>U2::uHMMPlugin</name>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="80"/>
        <source>HMM2</source>
        <translation>HMM2</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="80"/>
        <source>Based on HMMER 2.3.2 package. Biological sequence analysis using profile hidden Markov models</source>
        <translation>Основан на пакете HMMER 2.3.2: анализ биологических последовательностей при помощи скрытых марковских моделей (HMM).
Домашняя страница проекта: http://hmmer.janelia.org/.
Включает в себя оригинальные оптимизированные версии алгоритмов</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="83"/>
        <source>Build HMM2 profile...</source>
        <translation>Построение профиля с помощью HMMER2...</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="88"/>
        <source>Calibrate profile with HMMER2...</source>
        <translation>Калибровка профиля с помощью HMMER2...</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="93"/>
        <source>Search with HMMER2...</source>
        <translation>Поиск с помощью HMMER2...</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="190"/>
        <source>Error</source>
        <translation>Ошибка</translation>
    </message>
    <message>
        <location filename="../src/uHMMPlugin.cpp" line="190"/>
        <source>Error! Select sequence in Project view or open sequence view.</source>
        <translation>Сначала выберите последовательность.</translation>
    </message>
</context>
</TS>
