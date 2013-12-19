<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru">
<context>
    <name>U2::AddPluginTask</name>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="378"/>
        <source>Add plugin task: %1</source>
        <translation>Загрузка внешнего модуля: %1</translation>
    </message>
    <message>
        <source>plugin_file_not_found_%1</source>
        <translation type="obsolete">Не найден внешний модуль: %1</translation>
    </message>
    <message>
        <source>plugin_invalid_file_format_%1</source>
        <translation type="obsolete">Некорректный файл библиотеки: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="385"/>
        <source>Plugin is already loaded: %1</source>
        <translation>Модуль уже загружен: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="393"/>
        <source>Plugin %1 depends on %2 which is not loaded</source>
        <translation>Модуль %1 зависит от модуля %2, который не был загружен</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="397"/>
        <source>Plugin %1 depends on %2 which is available, but the version is too old</source>
        <translation>Модуль %1 зависит от модуля %2, который доступен, но версия которого устарела</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="408"/>
        <source>Plugin loading error: %1, Error string %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Plugin loading error: %1</source>
        <translation type="obsolete">Ошибка инициализации модуля: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="415"/>
        <source>Plugin initialization routine was not found: %1</source>
        <translation>Не найдена входная функция библиотеки модуля: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="421"/>
        <source>Plugin initialization failed: %1</source>
        <translation>Ошибка инициализации модуля: %1</translation>
    </message>
</context>
<context>
    <name>U2::ConsoleLogDriver</name>
    <message>
        <location filename="../src/ConsoleLogDriver.cpp" line="80"/>
        <source>Specifies the format of a log line.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ConsoleLogDriver.cpp" line="81"/>
        <source>Specifies the format of a log line.

Use the following notations: L - level, C - category, YYYY or YY - year, MM - month, dd - day, hh - hour, mm - minutes, ss - seconds, zzz - milliseconds. 

By default, logformat=&quot;[L][hh:mm]&quot;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ConsoleLogDriver.cpp" line="86"/>
        <source>&quot;&lt;format_string&gt;&quot;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ConsoleLogDriver.cpp" line="90"/>
        <source>Sets the log level.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ConsoleLogDriver.cpp" line="91"/>
        <source>Sets the log level per category. If a category is not specified, the log level is applied to all categories.

The following categories are available: 
&quot;Algorithms&quot;, &quot;Console&quot;, &quot;Core Services&quot;, &quot;Input/Output&quot;, &quot;Performance&quot;, &quot;Remote Service&quot;, &quot;Scripts&quot;, &quot;Tasks&quot;.

The following log levels are available: TRACE, DETAILS, INFO, ERROR or NONE.

By default, loglevel=&quot;ERROR&quot;.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ConsoleLogDriver.cpp" line="98"/>
        <source>&quot;&lt;category1&gt;=&lt;level1&gt; [&lt;category2&gt;=&lt;level2&gt; ...]&quot; | &lt;level&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/ConsoleLogDriver.cpp" line="101"/>
        <source>Enables colored output.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::DisableServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="283"/>
        <source>Disable &apos;%1&apos; service</source>
        <translation>Остановка сервиса: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="302"/>
        <source>Active top-level task name: %1</source>
        <translation>Активная задача: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="304"/>
        <source>Active task was found</source>
        <translation>Имеются неоконченные задачи</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="308"/>
        <source>Service is not registered: %1</source>
        <translation>Сервис не зарегистрирован: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="312"/>
        <source>Service is not enabled: %1</source>
        <translation>Сервис не запущен: %1</translation>
    </message>
</context>
<context>
    <name>U2::DocumentFormatRegistryImpl</name>
    <message>
        <location filename="../src/DocumentFormatRegistryImpl.cpp" line="204"/>
        <source>UGENE Database</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::EnableServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="183"/>
        <source>Service is already enabled: %1</source>
        <translation>Сервис запущен: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="189"/>
        <source>Circular service dependency: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <source>Circullar service dependency: %1</source>
        <translation type="obsolete">Циркулярная зависимость модуля: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="195"/>
        <source>Required service is not enabled: %1</source>
        <translation>Не запущен обеспечивающий зависимость сервис: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="170"/>
        <source>Enable &apos;%1&apos; service</source>
        <translation>Запуск сервиса: %1</translation>
    </message>
</context>
<context>
    <name>U2::LoadAllPluginsTask</name>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="96"/>
        <source>Loading start up plugins</source>
        <translation>Загрузка подключаемых модулей</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="126"/>
        <source>File not found: %1</source>
        <translation>Не найден внешний модуль: %1</translation>
    </message>
    <message>
        <location filename="../src/PluginSupportImpl.cpp" line="131"/>
        <source>Invalid file format: %1</source>
        <translation>Некорректный файл библиотеки: %1</translation>
    </message>
</context>
<context>
    <name>U2::LogCategories</name>
    <message>
        <location filename="../src/LogSettings.cpp" line="37"/>
        <source>TRACE</source>
        <translation>Трассировка</translation>
    </message>
    <message>
        <location filename="../src/LogSettings.cpp" line="38"/>
        <source>DETAILS</source>
        <translation>Детали</translation>
    </message>
    <message>
        <location filename="../src/LogSettings.cpp" line="39"/>
        <source>INFO</source>
        <translation>Информация</translation>
    </message>
    <message>
        <location filename="../src/LogSettings.cpp" line="40"/>
        <source>ERROR</source>
        <translation>Ошибки</translation>
    </message>
</context>
<context>
    <name>U2::PluginDescriptorHelper</name>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="109"/>
        <location filename="../src/PluginDescriptor.cpp" line="115"/>
        <location filename="../src/PluginDescriptor.cpp" line="121"/>
        <location filename="../src/PluginDescriptor.cpp" line="127"/>
        <source>Required attribute not found %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="138"/>
        <location filename="../src/PluginDescriptor.cpp" line="149"/>
        <location filename="../src/PluginDescriptor.cpp" line="155"/>
        <source>Required element not found %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="162"/>
        <source>Not valid value: &apos;%1&apos;, plugin: %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="170"/>
        <source>Platform arch is unknown: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="177"/>
        <source>Platform bits is unknown: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="196"/>
        <source>Invalid depends token: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="256"/>
        <source>Plugin circular dependency detected: %1 &lt;-&gt; %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/PluginDescriptor.cpp" line="340"/>
        <source>Can&apos;t satisfy dependencies for %1 !</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>U2::RegisterServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="139"/>
        <source>Register &apos;%1&apos; service</source>
        <translation>Регистрация сервиса: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="147"/>
        <source>Service is already registered &apos;%1&apos;</source>
        <translation>Сервис уже зарегистрирован: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="151"/>
        <source>Illegal service state: %1</source>
        <translation>Ошибка сервиса: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="157"/>
        <source>Only one service of specified type is allowed: %1</source>
        <translation>Разрешён только один экземпляр сервиса: %1</translation>
    </message>
</context>
<context>
    <name>U2::TaskSchedulerImpl</name>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="79"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="635"/>
        <source>New</source>
        <translation>Новая</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="79"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="637"/>
        <source>Prepared</source>
        <translation>Инициализирована</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="79"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="639"/>
        <source>Running</source>
        <translation>Выполняется</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="79"/>
        <location filename="../src/TaskSchedulerImpl.cpp" line="641"/>
        <source>Finished</source>
        <translation>Завершена</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="95"/>
        <source>Canceling task: %1</source>
        <translation>Отменяется задача &quot;%1&quot;</translation>
    </message>
    <message>
        <source>Subtask {%1} is canceled</source>
        <translation type="obsolete">Подзадача {%1} отменена.</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="129"/>
        <source>Subtask {%1} is failed: %2</source>
        <translation>Подзадача {%1} завершена с ошибкой: %2</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="349"/>
        <source>Waiting for resource &apos;%1&apos;, count: %2</source>
        <translation>Ожидание ресурса &quot;%1&quot;, требуется: %2</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="372"/>
        <source>Not enough resources for the task, resource name: &apos;%1&apos; max: %2%3 requested: %4%5</source>
        <translation>Не достаточно ресурсов для выполнения задачи: &apos;%1&apos; макс: %2%3	запрошено: %4%5</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="375"/>
        <source>Waiting for resource &apos;%1&apos;, count: %2%3</source>
        <translation>Ожидание ресурса &quot;%1&quot;, требуется: %2%3</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="505"/>
        <source>Registering new task: %1</source>
        <translation>Добавлена задача &quot;%1&quot;</translation>
    </message>
    <message>
        <source>Uregistering task: %1</source>
        <translation type="obsolete">Задача удаляется из списка: &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="644"/>
        <source>Invalid name</source>
        <translation>Неизвестное состояние</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="858"/>
        <source>Deleting task: %1</source>
        <translation>Удаляется задача &quot;%1&quot;</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="710"/>
        <source>Promoting task {%1} to &apos;%2&apos;</source>
        <translation>Задача {%1} %2.</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="121"/>
        <source>Subtask {%1} is canceled %2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="364"/>
        <source>No required resources for the task, resource id: &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="365"/>
        <source>Unable to run test because required resource not found</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="561"/>
        <source>Unregistering task: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="712"/>
        <source>Promoting task {%1} to &apos;%2&apos;, error &apos;%3&apos;</source>
        <translation>Задача {%1} %2; ошибка: %3</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="722"/>
        <source>Starting {%1} task</source>
        <translation>Старт задачи {%1}</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="748"/>
        <source>Task {%1} finished with error: %2</source>
        <translation>Задача {%1} завершена с ошибкой: %2</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="752"/>
        <source>Task {%1} canceled</source>
        <translation>Отменяется задача {%1}</translation>
    </message>
    <message>
        <location filename="../src/TaskSchedulerImpl.cpp" line="755"/>
        <source>Task {%1} finished</source>
        <translation>Задача {%1} завершена</translation>
    </message>
</context>
<context>
    <name>U2::UnregisterServiceTask</name>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="259"/>
        <source>Service is not registered: %1</source>
        <translation>Сервис не зарегистрирован: %1</translation>
    </message>
    <message>
        <location filename="../src/ServiceRegistryImpl.cpp" line="252"/>
        <source>Unregister &apos;%1&apos; service</source>
        <translation>Дерегистрация сервиса &quot;%1&quot;</translation>
    </message>
</context>
</TS>
