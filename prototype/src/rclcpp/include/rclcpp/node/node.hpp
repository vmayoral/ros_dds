#ifndef NODE_HPP
#define NODE_HPP
#include <map>

#include <boost/shared_ptr.hpp>

#include <ccpp_dds_dcps.h>

#include <rclcpp/publisher/publisher.hpp>
#include <rclcpp/subscription/subscription.hpp>

#include <genidlcpp/resolver.h>

namespace rclcpp
{

using publisher::Publisher;
using publisher::PublisherInterface;
using subscription::Subscription;

namespace node
{

class Node
{
public:
    Node(std::string name);
    ~Node();

    template <typename ROSMsgType>
    Publisher<ROSMsgType> create_publisher(std::string topic_name, size_t queue_size)
    {
        typedef typename dds_impl::DDSTypeResolver<ROSMsgType>::DDSMsgType DDSMsg_t;
        typedef typename dds_impl::DDSTypeResolver<ROSMsgType>::DDSMsgTypeSupportType DDSMsgTypeSupport_t;
        typedef typename dds_impl::DDSTypeResolver<ROSMsgType>::DDSMsgTypeSupportType_var DDSMsgTypeSupport_var;
        typedef typename dds_impl::DDSTypeResolver<ROSMsgType>::DDSMsgDataWriterType DDSMsgDataWriter_t;
        typedef typename dds_impl::DDSTypeResolver<ROSMsgType>::DDSMsgDataWriterType_var DDSMsgDataWriter_var;

        // TODO check return status
        DDS::ReturnCode_t status;

        DDSMsgTypeSupport_var dds_msg_ts = new DDSMsgTypeSupport_t();
        // checkHandle(dds_msg_ts.in(), "new DDSMsgTypeSupport");
        char * dds_msg_name = dds_msg_ts->get_type_name();
        status = dds_msg_ts->register_type(this->participant_.in(), dds_msg_name);
        // checkStatus(status, "TypeSupport::register_type");

        DDS::Publisher_var dds_publisher = this->participant_->create_publisher(
            this->default_publisher_qos_, NULL, DDS::STATUS_MASK_NONE);
        // checkHandle(dds_publisher.in(), "DDS::DomainParticipant::create_publisher");

        DDS::Topic_var dds_topic = this->participant_->create_topic(
            topic_name.c_str(), dds_msg_name, this->default_topic_qos_, NULL,
            DDS::STATUS_MASK_NONE
        );
        // checkHandle(dds_topic.in(), "DDS::DomainParticipant::create_topic");

        DDS::DataWriter_var dds_topic_datawriter = dds_publisher->create_datawriter(
            dds_topic.in(), DATAWRITER_QOS_USE_TOPIC_QOS,
            NULL, DDS::STATUS_MASK_NONE);
        // checkHandle(dds_topic_datawriter.in(), "DDS::Publisher::create_datawriter");

        if (this->publishers_.find(topic_name) != this->publishers_.end())
        {
            // TODO Raise, already called for topic
        }
        // boost::shared_ptr<PublisherInterface> publisher(
            // new Publisher<ROSMsgType>(topic_name, queue_size, dds_publisher, dds_topic, dds_topic_datawriter));

        // this->publishers_.inse/rt(std::pair<std::string, boost::shared_ptr<PublisherInterface> >(topic_name, publisher));
        // return *(dynamic_cast<const Publisher<ROSMsgType> *>(this->publishers_.at(topic_name).get()));
        return Publisher<ROSMsgType>(topic_name, queue_size, dds_publisher, dds_topic, dds_topic_datawriter);
    }

    void destroy_publisher(PublisherInterface * publisher);
    void destroy_publisher(std::string topic_name);

    template <typename T>
    Subscription<T> create_subscription(std::string topic_name,
                                        size_t queue_size,
                                        typename Subscription<T>::CallbackType cb);

    template <typename T>
    void destroy_subscription(Subscription<T> subscription);
private:
    std::string name_;
    DDS::DomainParticipantFactory_var dpf_;
    DDS::DomainParticipant_var participant_;
    DDS::TopicQos default_topic_qos_;
    DDS::PublisherQos default_publisher_qos_;

    std::map<std::string, PublisherInterface* > publishers_;
};

}
}
#endif
