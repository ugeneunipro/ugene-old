// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#ifndef BALL_COMMON_LOGSTREAM_H
#define BALL_COMMON_LOGSTREAM_H

#ifndef BALL_CONFIG_CONFIG_H
#	include <BALL/CONFIG/config.h>
#endif

#ifndef BALL_COMMON_GLOBAL_H
#	include <BALL/COMMON/global.h>
#endif

#ifndef BALL_COMMON_DEBUG_H
#	include <BALL/COMMON/debug.h>
#endif

#ifdef BALL_HAS_SYS_TIME_H
#	include <sys/time.h>
#endif

#ifdef BALL_HAS_TIME_H
#	include <time.h>
#endif

#ifdef BALL_HAS_SSTREAM
# include <sstream>
#else
# include <strstream>
#endif

#include <iostream>
#include <list>
#include <vector>
#include <string>


using std::list;
using std::vector;
using std::string;

namespace BALL 
{

	/**	@name Log streams
			Logging, filtering, and storing messages.
			Many programs emit warning messages, error messages, or simply
			informations and remarks to their users. The  \link LogStream LogStream \endlink 
			class provides a convenient and straight-forward interface 
			to classify these messages according to their importance 
			(via the loglevel), filter and store them in files or
			write them to streams. \par
			As the LogStream class is derived from ostream, it behaves 
			as any ostream object. Additionally you may associate
			streams with each LogStream object that catch only 
			messages of certain loglevels. So the user might decide to
			redirect all error messages to cerr, all warning messages
			to cout and all informations to a file. \par
			Along with each message its time of creation and its loglevel
			is stored. So the user might also decide to store all 
			errors he got in the last two hours or alike. \par
			The  \link LogStream LogStream \endlink  class heavily relies on the  \link LogStreamBuf LogStreamBuf \endlink 
			class, which does the actual buffering and storing, but is only
			of interest if you want to implement a derived class, as the 
			actual user interface is implemented in the LogStream class.
	* 	\ingroup Common
	*/
	//@{

	// forward declarations
	class LogStream;
	class LogStreamNotifier;

	/** Stream buffer used by LogStream.
			This class implements the low level behaviour of
			 \link LogStream LogStream \endlink . It takes care of the buffers and stores
			the lines written into the  \link LogStream LogStream \endlink  object.
			It also contains a list of streams that are associated with
			the LogStream object. This list contains pointers to the
			streams and their minimum and maximum log level.
			Each line entered in the  \link LogStream LogStream \endlink  is marked with its
			time (in fact, the time  \link LogStreamBuf::sync sync \endlink  was called) and its
			loglevel. The loglevel is determined by either the current
			loglevel (as set by  \link LogStream::setLevel LogStream::setLevel \endlink  or a temporary
			level (as set by  \link LogStream::level LogStream::level \endlink  for a single line only).
			For each line stored, the list of associated streams is checked
			whether the loglevel falls into the range declared by the 
			stream's minimum and maximum level. If this condition is met,
			the logline (with its prefix, see  \link LogStream::setPrefix LogStream::setPrefix \endlink )
			is also copied to the associated stream and this stream is 
			flushed, too.
	*/
	class BALL_EXPORT LogStreamBuf
		: public std::streambuf
	{

		friend class LogStream;

		public:

		/**	@name	Constants
		*/
		//@{
		static const int MAX_LEVEL;
		static const int MIN_LEVEL;
		static const Time MAX_TIME;
		//@}

		/**	@name Constructors and Destructors
		*/
		//@{
		
		/** Default constructor.
				Create a new LogStreamBuf object.
		*/
		LogStreamBuf();

		/** Destructor.
				Destruct the buffer and free all stored messages strings.
		*/
		virtual ~LogStreamBuf();
		
		//@}
		

		/**	@name	Debugging and Diagnostics
		*/
		//@{
		
		/** Dump method.
				Dumps the contents of the whole message buffer 
				including time and log level.
		*/
		virtual void dump(std::ostream& s);

		//@}

		/**	@name	Stream methods 
		*/
		//@{

		/**	Sync method.
				This method is called as soon as the ostream is flushed
				(especially this method is called by flush or endl).
				It transfers the contents of the streambufs putbuffer 
				into a logline if a newline or linefeed character 
				is found in the buffer ("\n" or "\r" resp.).
				The line is then removed from the putbuffer.
				Incomplete lines (not terminated by "\n" / "\r" are
				stored in incomplete_line_.
		*/
		virtual int sync();

		int sync(bool force_flush);

		/**	Overflow method.
				This method calls sync and <tt>streambuf::overflow(c)</tt> to 
				prevent a buffer overflow.
		*/
		virtual int overflow(int c = -1);
		//@}

		BALL_EXPORT struct StreamStruct
		{
			std::ostream*				stream;
			string							prefix;
			int									min_level;
			int									max_level;
			LogStreamNotifier*	target;
			bool disabled;
		
			StreamStruct()
				:	stream(0),
					min_level(MIN_LEVEL),
					max_level(MAX_LEVEL),
					target(0)
			{
			}
			
			// Delete the notification target.
			~StreamStruct()
			{
			}
		};


		protected:

		struct LoglineStruct 
		{	
			int     level;
			string  text;
			Time  time;

			LoglineStruct()
				: level(0),
					text(""),
					time(0)
			{}
		};

		typedef struct LoglineStruct Logline;


		// interpret the prefix format string and return the expanded prefix
		string expandPrefix_(const string& prefix, int level, Time time) const;

		char* 									pbuf_;

		vector<Logline> 				loglines_;
	
		int											level_;

		int											tmp_level_;
		
		list<StreamStruct>			stream_list_;

		string									incomplete_line_;
	};


	///
	class BALL_EXPORT LogStreamNotifier
	{
		public:
		
		///
		LogStreamNotifier();
			
		///
		virtual ~LogStreamNotifier();

		///
		virtual void logNotify();

		///
		void registerAt(LogStream& log_stream,
										int min_level = LogStreamBuf::MIN_LEVEL, 
										int max_level = LogStreamBuf::MAX_LEVEL);
		///
		void unregister();

		protected:

		std::stringstream stream_;

		LogStream* registered_at_;
	};



	/**	Log Stream Class.
			 \par
			
			 \par
	*/
	class BALL_EXPORT LogStream
		: public std::ostream
	{
		public:


		/**	@name Enums
		*/
		//@{
			
		/** Log levels.
				Constants for the different predefined log levels.
				Use  \link LogStream::ERROR ERROR \endlink  to indicate a severe error,  \link LogStream::WARNING WARNING \endlink  to 
				indicate a problem that could be fixed or is of minor importance, 
				and  \link LogStream::INFORMATION INFORMATION \endlink  for messages that do not indicate any problem 
				(e.g. progress messages).
		*/
		enum LogStreamLevel
		{
			/** Loglevels >= ERROR should be used to indicate errors
			*/

			ERROR_LEVEL = 2000 ,
			
			/** Loglevels >= WARNING should be used to indicate warnings
			*/
			WARNING_LEVEL = 1000,
			/** Loglevels >= INFORMATION indicate information messages
			*/
			INFORMATION_LEVEL = 0
		};

		//@}
	
		/**	@name	Constructors and Destructors
		*/
		//@{

		/** Constructor.
				Creates a new LogStream object that is not associated with any stream.
				If the argument <tt>associate_stdio</tt> is set to <b>true</b>,
				<tt>cout</tt> is associated with all messages of levels  \link LogStream::INFORMATION INFORMATION \endlink  
				and  \link LogStream::WARNING WARNING \endlink , and <tt>cerr</tt> is associated with all messages
				of level  \link LogStream::ERROR ERROR \endlink .
				@param	buf
				@param  delete_buf
				@param	associate_stdio bool, default is false
		*/
		LogStream(LogStreamBuf* buf = 0, bool delete_buf = true, bool associate_stdio = false);

		/** Destructor.
				Clears all message buffers.
		*/
		virtual ~LogStream();
	
		//@}		

		/**	@name	Stream Methods
		*/
		//@{

		/**	<tt>rdbuf</tt> method of ostream.
				This method is needed to access the LogStreamBuf object.
		*/
		LogStreamBuf* rdbuf();

		/** Arrow operator.
		*/
		LogStreamBuf* operator -> ();
		//@}

		/**	@name Loglevel management 
		*/
		//@{

		/**	Assign a new log level.
				This method assigns a new loglevel which will be used
				for all messages sent to the LogStream after that call
				(except for messages which use the temporary loglevel
				set by  \link LogStream::level level \endlink ).
		*/
		void setLevel(int level);

		/**	Return the current log level.
				The LogStreamBuf object has an internal current log level (<tt>level_</tt>).
				It is set to 0 by the LogStreamBuf default constructor.
				This method returns <tt>rdbuf()->level_</tt> if rdbuf() does not
				return a null pointer, 0 otherwise.
				@return		int the current log level
		*/
		int getLevel();

		/**	Set a temporary log level.
				Using <b>level</b>, a temporary loglevel may be defined.
				It is valid unly until the next <b>flush</b> or <b>endl</b> is issued. \par
				Use this command to log a single line with a certain log level. \par
				<b>Example:</b>
					<tt>log << "log message 1" << endl;</tt> \par
					<tt>log.level(4) << "log message 2" << endl;</tt> \par
					<tt>log << "log message 3" << endl;</tt> \par
				In this example, only the second message will be logged using level 4.
				
				@return	LogStream the log stream
				@param	level the temporary log level
		*/
		LogStream& level(int level);

		/**	Log an information message.
				This method is equivalent to  \link LogStream::level level \endlink (LogStream::INFORMATION + n). 
				@param	n the channel 
		*/
		LogStream& info(int n = 0);

		/**	Log an error message.
				This method is equivalent to  \link LogStream::level level \endlink (LogStream::ERROR + n). 
				@param	n the channel 
		*/
		LogStream& error(int n = 0);

		/**	Log an information message.
				This method is equivalent to  \link LogStream::level level \endlink (LogStream::WARNING + n). 
				@param	n the channel 
		*/
		LogStream& warn(int n = 0);

		//@}

		/**	@name Associating Streams 
		*/
		//@{

		/**	Associate a new stream with this logstream.
				This method inserts a new stream into the list of 
				associated streams and sets the corresponding minimum
				and maximum log levels.
				Any message that is subsequently logged, will be copied
				to this stream if its log level is between <tt>min_level</tt>
				and <tt>max_level</tt>. If <tt>min_level</tt> and <tt>max_level</tt>
				are omitted, all messages are copied to this stream.
				If <tt>min_level</tt>	and <tt>max_level</tt> are equal, this function can be used
				to listen to a specified channel.
				@param	s a reference to the stream to be associated
				@param	MIN_LEVEL the minimum level of messages copied to this stream
				@param	MAX_LEVEL the maximum level of messages copied to this stream
		*/
		void insert
			(std::ostream& s, int min_level = LogStreamBuf::MIN_LEVEL, 
			 int max_level = LogStreamBuf::MAX_LEVEL);

		/**	Remove an association with a stream.
				Remove a stream from the stream list and avoid the copying of new messages to
				this stream. \par
				If the stream was not in the list of associated streams nothing will
				happen.
				@param	s the stream to be removed
		*/
		void remove(std::ostream& s);

		/**	Add a notification target
		*/
		void insertNotification(std::ostream& s, 
														LogStreamNotifier& target,
														int min_level = LogStreamBuf::MIN_LEVEL, 
														int max_level = LogStreamBuf::MAX_LEVEL);

		/**	Set the minimum log level of an associated stream.
				This method changes the minimum log level of an already
				associated stream. However, if the stream is not
				associated, nothing will happen.
				@param	s the associated stream
				@param	min_level the new minimum level
		*/
		void setMinLevel(const std::ostream& s, int min_level);
		
		/**	Set the maximum log level of an associated stream.
				This method changes the maximum log level of an already
				associated stream. However, if the stream is not
				associated, nothing will happen.
				@param	s the associated stream
				@param	max_level the new minimum level
		*/
		void setMaxLevel(const std::ostream& s, int max_level);

		/**	Set prefix for output to this stream.
				Each line written to the stream will be prefixed by
				this string. The string may also contain trivial 
				format specifiers to include loglevel and time/date 
				of the logged message. \par
				The following format tags are recognized:

					- <b>%l</b>	loglevel
					- <b>%y</b>	message type ("Error", "Warning", "Information", "-")
					- <b>%T</b>  time (HH:MM:SS)
					- <b>%t</b>  time in short format (HH:MM)
					- <b>%D</b>	date (DD.MM.YYYY)
					- <b>%d</b>  date in short format (DD.MM.)
					- <b>%S</b>  time and date (DD.MM.YYYY, HH:MM:SS)
					- <b>%s</b>  time and date in short format (DD.MM., HH:MM)
					- <b>%%</b>	percent sign (escape sequence)
				
		*/
		void setPrefix(const std::ostream& s, const string& prefix);

		/// Disable all output
		void disableOutput() ;

		/// Enable all output
		void enableOutput() ;

		/// Is Output enabled?
		bool outputEnabled() const
			;

		/** Flush the LogStream.\n
		This function overloads std::ostream::flush(). */
		std::ostream& flush();

		//@}		
		
		/**	@name	Message Buffer Management */
		//@{
			
		/** Clear the message buffer.
				This method removes all stored messages from the 
				message buffer.
		*/
		void clear();
	
		/**	Return the number of lines.
				This method retruns the number of lines in the buffer 
				for a given range of levels. \par
				If the range is omitted, the total number of messages is
				returned.
				@return Size the number of lines matching the log level range
				@param	min_level the minimum log level for the counted messages
				@param	max_level the maximum log level for the counted messages
		*/
		Size getNumberOfLines
			(int min_level = LogStreamBuf::MIN_LEVEL, 
			 int max_level = LogStreamBuf::MAX_LEVEL) const;

		/**	Return the text of a specific line.
				This method returns the content of a specific message without
				time and level.
				@return string the text of the message
				@param	index the index of the line
		*/
		string getLineText(const Index& index) const;

		/**	Return the log time of a specific line
				@param index the index of the messages
				@return Time the time of the message
		*/
		Time getLineTime(const Index& index) const;	
	
		/**	Return the log level of a specific line.
				If the given line does not exists, {\em -1} is returned.
				@param index the index of the message
				@return int the level
		*/
		int getLineLevel(const Index& index) const;
		
		/** Retrieve a list of indices of lines that match certain criteria.
				If a criterion is left empty, it is not used.
				@param min_level the minimum level of messages
				@param max_level the maximum level of messages
				@param earliest (long) the time of messages to start filtering
				@param latest (long) the time of messages to stop filtering
				@param s a string to look for
		*/
		list<int>	filterLines
			(int min_level = LogStreamBuf::MIN_LEVEL, int max_level = LogStreamBuf::MAX_LEVEL,
			 Time earliest = 0, Time latest = LogStreamBuf::MAX_TIME, 
			 const string& s = "") const;
		//@}

		private:

		typedef std::list<LogStreamBuf::StreamStruct>::iterator StreamIterator;
		
		StreamIterator findStream_(const std::ostream& stream);
		bool hasStream_(std::ostream& stream);
		bool bound_() const;

		// flag needed by the destructor to decide whether the streambuf
		// has to be deleted. If the default ctor is used to create
		// the LogStreamBuf, delete_buffer_ is set to true and the ctor
		// also deletes the buffer.
		bool	delete_buffer_;
		bool  disable_output_;
	};


	/** Global static instance of a logstream.
			This instance of LogStream is by default bound to <b>cout</b> <b>cerr</b> by calling
			the default constructor.
	*/
	BALL_EXPORT extern LogStream	Log;

	//@}
	
#	ifndef BALL_NO_INLINE_FUNCTIONS
#		include <BALL/COMMON/logStream.iC>
#	endif

} // namespace BALL

#endif // BALL_COMMON_LOGSTREAM_H
