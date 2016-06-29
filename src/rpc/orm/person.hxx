#ifndef PERSON_HXX
#define PERSON_HXX

#include <string>
#include <cstddef>

#include <memory>
#include <odb/core.hxx>
#include <odb/nullable.hxx>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>
#include <odb/boost/exception.hxx>
#include <odb/boost/date-time/exceptions.hxx>
#include <boost/date_time/posix_time/posix_time.hpp>

#pragma db model version(1, 1)
#pragma db object optimistic
#pragma db object session
class person {
private:
    friend class odb::access;
    person(){}

    #pragma db id auto
    unsigned long id_;
    #pragma db version
    unsigned long version_;

    std::string first_;

	#pragma db default("")
    std::string last_;

    unsigned short age_;

    //boost::optional<std::string> middle_;// TEXT NULL
	#pragma db null
	boost::shared_ptr<std::string> middle_name_;

	#pragma db type("TIMESTAMP") not_null
	boost::posix_time::ptime created_time_;

	#pragma db type("DATETIME(6)")// Microsecond precision.
	boost::posix_time::ptime updated_time_;

public:
    person (const std::string& first,
        const std::string& last,
        unsigned short age,
		const boost::shared_ptr<std::string>& middle_name,
		const boost::posix_time::ptime& created_time,
		const boost::posix_time::ptime& updated_time
		)
      : first_ (first), last_ (last), age_ (age),middle_name_(middle_name)
		,created_time_(created_time),updated_time_(updated_time)
    {
    }

    const std::string& first() const {return first_;}
    void first(const std::string & first){first_ = first;}

    const std::string& last() const{return last_;}
    void last(const std::string & last){last_ = last;}

    const boost::shared_ptr<std::string>& middle_name() const{return middle_name_;}
    void middle_name(const boost::shared_ptr<std::string> & middle_name){middle_name_ = middle_name;}

    unsigned short age() const{return age_;}
    void age(unsigned short age){age_ = age;}

    const boost::posix_time::ptime& created_time() const {return created_time_;}
    void created_time(const boost::posix_time::ptime & created_time){created_time_ = created_time;}

    const boost::posix_time::ptime& updated_time() const {return updated_time_;}
	void updated_time(const boost::posix_time::ptime & updated_time){updated_time_ = updated_time;}

};

#pragma db view object(person)
struct person_stat
{
    #pragma db column("count(" + person::id_ + ")")
    std::size_t count;

    #pragma db column("min(" + person::age_ + ")")
    unsigned short min_age;

    #pragma db column("max(" + person::age_ + ")")
    unsigned short max_age;
};

#endif
