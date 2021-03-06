/* CreateTopics.c */
#include "dds_dcps.h"
#include "Chat.h"
#include "unistd.h"

int main (
  int argc,
  char *argv[])
{
  DDS_DomainParticipantFactory dpf;
  DDS_DomainParticipant dp;
  DDS_DomainId_t domain = DDS_DOMAIN_ID_DEFAULT;
  DDS_ReturnCode_t status;
  Chat_ChatMessageTypeSupport chatMessageTS;
  DDS_Topic chatMessageTopic;
  char *chatMessageTypeName;

  /* Create a DomainParticipantFactory and a DomainParticipant */
  /* (using Default QoS settings). */
  dpf = DDS_DomainParticipantFactory_get_instance();
  if (!dpf) {
    printf("Creating ParticipantFactory failed!!\n");
    exit(-1);
  }
  printf("Created ParticipantFactory.\n");

  dp = DDS_DomainParticipantFactory_create_participant (
    dpf,
    domain,
    DDS_PARTICIPANT_QOS_DEFAULT,
    NULL,
    DDS_STATUS_MASK_NONE);
  if (!dp) {
    printf("Creating Participant failed!!\n");
    exit(-1);
  }
  printf("Created Participant.\n");


  /* Register the required data type for ChatMessage. */
  chatMessageTS = Chat_ChatMessageTypeSupport__alloc();
  if (!chatMessageTS) {
    printf ("Allocating TypeSupport failed!!\n");
    exit(-1);
  };
  chatMessageTypeName = Chat_ChatMessageTypeSupport_get_type_name(chatMessageTS);
  status = Chat_ChatMessageTypeSupport_register_type(
    chatMessageTS, dp, chatMessageTypeName);
  if (status != DDS_RETCODE_OK) {
    printf("Registering data type failed. Status = %d\n", status);
    exit(-1);
  };
  printf("Registered data type.\n");

  /*Create the ChatMessage topic */
  chatMessageTopic = DDS_DomainParticipant_create_topic(
    dp,
    "Chat_ChatMessage",
    chatMessageTypeName,
    DDS_TOPIC_QOS_DEFAULT,
    NULL,
    DDS_STATUS_MASK_NONE);
  if (!chatMessageTopic) {
    printf("Creating ChatMessage topic failed!!\n");
    exit(-1);
  };
  printf("Created ChatMessage topic.\n");

  DDS_PublisherQos *pub_qos;
  DDS_DataWriterQos *dw_qos;
  DDS_Publisher chatPublisher;
  Chat_ChatMessageDataWriter talker;
  Chat_NameServiceDataWriter nameServer;
  char *partitionName = NULL;

  DDS_Topic nameServiceTopic;

  /* Adapt the default PublisherQos to write into the
     "ChatRoom" Partition. */
  partitionName = "ChatRoom";
  pub_qos = DDS_PublisherQos__alloc();
  if (!pub_qos) {
    printf("Allocating PublisherQos failed!!\n");
    exit(-1);
  }
  status = DDS_DomainParticipant_get_default_publisher_qos (
    dp, pub_qos);
  if (status != DDS_RETCODE_OK) {
    printf("Getting default publisher qos failed!!\n");
    exit(-1);
  }
  pub_qos->partition.name._length = 1;
  pub_qos->partition.name._maximum = 1;
  pub_qos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
  if (!pub_qos->partition.name._buffer) {
    printf("Allocating partition name failed!!\n");
    exit(-1);
  }
  pub_qos->partition.name._buffer[0] = DDS_string_alloc (
    strlen(partitionName));
  if (!pub_qos->partition.name._buffer[0]) {
    printf("Allocating partition name failed!!\n");
    exit(-1);
  }
  strcpy (pub_qos->partition.name._buffer[0], partitionName);

  /* Create a Publisher for the chatter application. */
  chatPublisher = DDS_DomainParticipant_create_publisher(
    dp, pub_qos, NULL, DDS_STATUS_MASK_NONE);
  if (!chatPublisher) {
    printf("Creating publisher failed!!\n");
    exit(-1);
  }
  printf("Created publisher.\n");

  /* Create a DataWriter for the ChatMessage Topic
     (using the appropriate QoS). */
  talker = DDS_Publisher_create_datawriter(
    chatPublisher,
    chatMessageTopic,
    DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
    NULL,
    DDS_STATUS_MASK_NONE);
  if (!talker) {
    printf("Creating datawriter failed!!\n");
    exit(-1);
  }
  printf("Created datawriter.\n");

  /* Create a DataWriter for the NameService Topic
     (using the appropriate QoS). */
  /*dw_qos = DDS_DataWriterQos__alloc();
  if (!dw_qos) {
    printf("Allocating datawriter QoS failed!!\n");
    exit(-1);
  }
  status = DDS_Publisher_get_default_datawriter_qos(
    chatPublisher, dw_qos);
  if (status != DDS_RETCODE_OK) {
    printf("Getting default datawriter qos failed!!\n");
    exit(-1);
  }
  //status = DDS_Publisher_copy_from_topic_qos(
  //  chatPublisher, dw_qos, setting_topic_qos);
  //checkStatus(status, "DDS_Publisher_copy_from_topic_qos");
  dw_qos->writer_data_lifecycle.autodispose_unregistered_instances = FALSE;
  nameServer = DDS_Publisher_create_datawriter(
    chatPublisher,
    nameServiceTopic,
    dw_qos,
    NULL,
    DDS_STATUS_MASK_NONE);
  if (!nameServer) {
    printf("Creating datawriter (NameService) failed!!\n");
    exit(-1);
  }
  printf("Created datawriter (NameService).\n");*/

  sleep(5);


  /* Remove the DataWriters */
  status = DDS_Publisher_delete_datawriter(chatPublisher,
    talker);
  if (status != DDS_RETCODE_OK) {
    printf("Deleting datawriter failed!!\n");
    exit(-1);
  }
  printf("Deleted datawriter.\n");
  /*status = DDS_Publisher_delete_datawriter(
    chatPublisher, nameServer);
  if (status != DDS_RETCODE_OK) {
    printf("Deleting datawriter (NameService) failed!!\n");
    exit(-1);
  }
  printf("Deleted datawriter (NameService).\n");*/

  /* Remove the Publisher. */
  status = DDS_DomainParticipant_delete_publisher(
    dp, chatPublisher);
  if (status != DDS_RETCODE_OK) {
    printf("Deleting publisher failed!!\n");
    exit(-1);
  }
  /* De-allocate the PublisherQoS holder. */
  DDS_free(pub_qos); // Note that DDS_free recursively
                     // de-allocates all indirections!!
  printf("Deleted publisher.\n");

  /* Deleting the Topic. */
  status = DDS_DomainParticipant_delete_topic(
    dp, chatMessageTopic);
  if (status != DDS_RETCODE_OK) {
    printf("Deleting topic failed. Status = %d\n", status);
    exit(-1);
  };
  printf("Deleted ChatMessage topic.\n");

  /* Deleting the DomainParticipant */
  status = DDS_DomainParticipantFactory_delete_participant(
  dpf, dp);
  if (status != DDS_RETCODE_OK) {
    printf("Deleting participant failed. Status = %d\n", status);
    exit(-1);
  };
  printf("Deleted Participant.\n");

  /* Everything is fine, return normally. */
  return 0;
};
