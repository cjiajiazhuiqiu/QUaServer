#ifndef QUASERVER_H
#define QUASERVER_H

#include <type_traits>

#include <QUaTypesConverter>
#include <QUaFolderObject>
#include <QUaBaseDataVariable>
#include <QUaProperty>
#include <QUaBaseObject>
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
#include <QUaBaseEvent>
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

class QUaServer : public QObject
{
	friend class QUaNode;
	friend class QUaBaseVariable;
	friend class QUaBaseObject;
#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	friend class QUaBaseEvent;
#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS
	template <typename ClassType, typename R, bool IsMutable, typename... Args> friend struct QUaMethodTraitsBase;

	Q_OBJECT

public:

#ifndef UA_ENABLE_ENCRYPTION
	explicit QUaServer(const quint16    &intPort         = 4840, 
		               const QByteArray &byteCertificate = QByteArray(), 
		               QObject          *parent          = 0);
#else
	explicit QUaServer(const quint16    &intPort         = 4840, 
		               const QByteArray &byteCertificate = QByteArray(), 
		               const QByteArray &bytePrivateKey  = QByteArray(), 
		               QObject          *parent          = 0);
#endif
	
	~QUaServer();

	// Server Description API

	QString applicationName() const;
	void    setApplicationName(const QString &strApplicationName);
	QString applicationUri() const;
	void    setApplicationUri(const QString &strApplicationUri);
	QString productName() const;
	void    setProductName(const QString &strProductName);
	QString productUri() const;
	void    setProductUri(const QString &strProductUri);
	QString manufacturerName() const;
	void    setManufacturerName(const QString &strManufacturerName);
	QString softwareVersion() const;
	void    setSoftwareVersion(const QString &strSoftwareVersion);
	QString buildNumber() const;
	void    setBuildNumber(const QString &strBuildNumber);

	// Server LifeCycle API

	void start();
	void stop();
	bool isRunning() const;

	// Server Limits API

	quint16 maxSecureChannels() const;
	void    setMaxSecureChannels(const quint16 &maxSecureChannels);

	quint16 maxSessions() const;
	void    setMaxSessions(const quint16 &maxSessions);

	// Instance Creation API

	// register type in order to assign it a typeNodeId
	template<typename T>
	void registerType(const QString &strNodeId = "");
	// register enum in order to use it as data type
	template<typename T>
	void registerEnum(const QString &strNodeId = "");
	void registerEnum(const QString &strEnumName, const QMap<int, QByteArray> &mapEnum, const QString &strNodeId = "");
	// register reference to get a respective refTypeId
	void registerReference(const QUaReference &ref);

	// create instance of a given (variable or object) type
	template<typename T>
	T* createInstance(QUaNode * parentNode, const QString &strNodeId = "");
	// get objects folder
	QUaFolderObject * objectsFolder();
	// get node reference by node id and cast to type (nullptr if node id does not exist)
	template<typename T>
	T* nodeById(const QString &strNodeId);
	// get node reference by node id (nullptr if node id does not exist)
	QUaNode * nodeById(const QString &strNodeId);

	// Browse API
	// (* actually browses using QObject tree)

	template<typename T>
	T* browsePath(const QStringList &strBrowsePath);
	// specialization
	QUaNode * browsePath(const QStringList &strBrowsePath);

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS
	// Events API

	// create instance of a given event type
	template<typename T>
	T* createEvent();

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// Access Control API

	// anonymous login is enabled by default
	bool        anonymousLoginAllowed() const;
	void        setAnonymousLoginAllowed(const bool &anonymousLoginAllowed) const;
	// if user already exists, it updates password
	void        addUser(const QString &strUserName, const QString &strPassword);
	// if user does not exist, it does nothing
	void        removeUser(const QString &strUserName);
	// number of users
	int         userCount();
	// get all user names
	QStringList userNames() const;
	// check if user already exists
	bool        userExists(const QString &strUserName) const;

signals:
	void iterateServer();

public slots:
	

private:
	UA_Server             * m_server;
	UA_Boolean              m_running;
	QByteArray              m_byteCertificate; // NOTE : needs to exists as long as server instance
	QMetaObject::Connection m_connection;
	QUaFolderObject       * m_pobjectsFolder;

#ifdef UA_ENABLE_ENCRYPTION
	QByteArray              m_bytePrivateKey; // NOTE : needs to exists as long as server instance
#endif

	QByteArray m_byteApplicationName;
	QByteArray m_byteApplicationUri;

	QByteArray m_byteProductName;
	QByteArray m_byteProductUri;
	QByteArray m_byteManufacturerName;
	QByteArray m_byteSoftwareVersion;
	QByteArray m_byteBuildNumber;

	QHash<QString     , QString  > m_hashUsers;
	QHash<UA_NodeId   , QString  > m_hashSessions;
	QMap <QString     , UA_NodeId> m_mapTypes;
	QHash<QString     , UA_NodeId> m_hashEnums;
	QHash<QUaReference, UA_NodeId> m_hashRefs;

	// only call once on constructor
	static UA_ByteString * parseCertificate(const QByteArray &inByteCert, 
		                                    UA_ByteString    &outUaCert, 
		                                    QByteArray       &outByteCert);
	void setupServer();

	void registerType(const QMetaObject &metaObject, const QString &strNodeId = "");
	void registerEnum(const QMetaEnum &metaEnum, const QString &strNodeId = "");

    void registerTypeLifeCycle(const UA_NodeId &typeNodeId, const QMetaObject &metaObject);
	void registerTypeLifeCycle(const UA_NodeId *typeNodeId, const QMetaObject &metaObject);

	void registerMetaEnums(const QMetaObject &parentMetaObject);
	void addMetaProperties(const QMetaObject &parentMetaObject);
	void addMetaMethods   (const QMetaObject &parentMetaObject);

	UA_NodeId createInstance(const QMetaObject &metaObject, QUaNode * parentNode, const QString &strNodeId = "");

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

	// create instance of a given event type
	UA_NodeId createEvent(const QMetaObject &metaObject, const UA_NodeId &nodeIdOriginator);

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

	void bindCppInstanceWithUaNode(QUaNode * nodeInstance, UA_NodeId &nodeId);

	QHash< UA_NodeId, std::function<UA_StatusCode(const UA_NodeId *nodeId, void ** nodeContext)>> m_hashConstructors;
	QHash< UA_NodeId, std::function<UA_StatusCode(void *, const UA_Variant*, UA_Variant*)>      > m_hashMethods;

	static UA_NodeId getReferenceTypeId(const QMetaObject &parentMetaObject, const QMetaObject &childMetaObject);

	static UA_StatusCode uaConstructor(UA_Server        *server,
		                               const UA_NodeId  *sessionId, 
		                               void             *sessionContext,
		                               const UA_NodeId  *typeNodeId, 
		                               void             *typeNodeContext,
		                               const UA_NodeId  *nodeId, 
		                               void            **nodeContext);

	static void uaDestructor         (UA_Server        *server,
		                              const UA_NodeId  *sessionId, 
		                              void             *sessionContext,
		                              const UA_NodeId  *typeNodeId, 
		                              void             *typeNodeContext,
		                              const UA_NodeId  *nodeId, 
		                              void            **nodeContext);

	static UA_StatusCode uaConstructor(QUaServer         *server,
		                               const UA_NodeId   *nodeId, 
		                               void             **nodeContext,
		                               const QMetaObject &metaObject);

	static UA_StatusCode methodCallback(UA_Server        *server,
		                                const UA_NodeId  *sessionId,
		                                void             *sessionContext,
		                                const UA_NodeId  *methodId,
		                                void             *methodContext,
		                                const UA_NodeId  *objectId,
		                                void             *objectContext,
		                                size_t            inputSize,
		                                const UA_Variant *input,
		                                size_t            outputSize,
		                                UA_Variant       *output);

	static bool isNodeBound(const UA_NodeId &nodeId, UA_Server *server);

	struct QOpcUaEnumValue
	{
		UA_Int64         Value;
		UA_LocalizedText DisplayName;
		UA_LocalizedText Description;
	};

	static QUaServer * getServerNodeContext(UA_Server * server);

	static UA_StatusCode createEnumValue(const QOpcUaEnumValue * enumVal, UA_ExtensionObject * outExtObj);

	static UA_StatusCode addEnumValues(UA_Server * server, UA_NodeId * parent, const UA_UInt32 numEnumValues, const QOpcUaEnumValue * enumValues);

	static UA_StatusCode activateSession(UA_Server                    *server, 
		                                 UA_AccessControl             *ac,
		                                 const UA_EndpointDescription *endpointDescription,
		                                 const UA_ByteString          *secureChannelRemoteCertificate,
		                                 const UA_NodeId              *sessionId,
		                                 const UA_ExtensionObject     *userIdentityToken,
		                                 void                        **sessionContext);

	static void closeSession(UA_Server        *server, 
		                     UA_AccessControl *ac, 
		                     const UA_NodeId  *sessionId, 
		                     void             *sessionContext);

	static UA_UInt32 getUserRightsMask(UA_Server        *server,
		                               UA_AccessControl *ac,
		                               const UA_NodeId  *sessionId,
		                               void             *sessionContext,
		                               const UA_NodeId  *nodeId,
		                               void             *nodeContext);

	static UA_Byte getUserAccessLevel(UA_Server        *server, 
		                              UA_AccessControl *ac,
		                              const UA_NodeId  *sessionId, 
		                              void             *sessionContext,
		                              const UA_NodeId  *nodeId, 
		                              void             *nodeContext);

	static UA_Boolean getUserExecutable(UA_Server        *server, 
		                                UA_AccessControl *ac,
		                                const UA_NodeId  *sessionId, 
		                                void             *sessionContext,
		                                const UA_NodeId  *methodId, 
		                                void             *methodContext);

	static UA_Boolean getUserExecutableOnObject(UA_Server        *server, 
		                                        UA_AccessControl *ac,
		                                        const UA_NodeId  *sessionId, 
		                                        void             *sessionContext,
		                                        const UA_NodeId  *methodId, 
		                                        void             *methodContext,
		                                        const UA_NodeId  *objectId, 
		                                        void             *objectContext);

	static void writeBuildInfo(UA_Server         *server, 
		                       const UA_NodeId    nodeId, 
		                       void * UA_RESTRICT pField, 
		                       void * UA_RESTRICT pBuild);

	// NOTE : temporary values needed to instantiate node, used to simplify user API
	//        passed-in in QUaServer::uaConstructor and used in QUaNode::QUaNode
	const UA_NodeId   * m_newNodeNodeId;
	const QMetaObject * m_newNodeMetaObject;
	//        passed-in in QUaServer::createEvent, QUaBaseObject::createEvent and used in QUaBaseEvent::QUaBaseEvent
	const UA_NodeId   * m_newEventOriginatorNodeId;
};

template<typename T>
inline void QUaServer::registerType(const QString &strNodeId/* = ""*/)
{
	// call internal method
	this->registerType(T::staticMetaObject, strNodeId);
}

template<typename T>
inline void QUaServer::registerEnum(const QString &strNodeId/* = ""*/)
{
	// call internal method
	this->registerEnum(QMetaEnum::fromType<T>(), strNodeId);
}

template<typename T>
inline T * QUaServer::createInstance(QUaNode * parentNode, const QString &strNodeId/* = ""*/)
{
	// instantiate first in OPC UA
	UA_NodeId newInstanceNodeId = this->createInstance(T::staticMetaObject, parentNode, strNodeId);
	if (UA_NodeId_isNull(&newInstanceNodeId))
	{
		return nullptr;
	}
	// get new c++ instance created in UA constructor
	auto tmp = QUaNode::getNodeContext(newInstanceNodeId, this);
	T * newInstance = dynamic_cast<T*>(tmp);
	Q_CHECK_PTR(newInstance);
	// return c++ instance
	return newInstance;
}

#ifdef UA_ENABLE_SUBSCRIPTIONS_EVENTS

template<typename T>
inline T * QUaServer::createEvent()
{
	// instantiate first in OPC UA
	UA_NodeId newEventNodeId = this->createEvent(T::staticMetaObject, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER));
	if (UA_NodeId_isNull(&newEventNodeId))
	{
		return nullptr;
	}
	// get new c++ instance created in UA constructor
	auto tmp = QUaNode::getNodeContext(newEventNodeId, this);
	T * newEvent = dynamic_cast<T*>(tmp);
	Q_CHECK_PTR(newEvent);
	// return c++ event instance
	return newEvent;
}

#endif // UA_ENABLE_SUBSCRIPTIONS_EVENTS

template<typename T>
inline T * QUaServer::nodeById(const QString &strNodeId)
{
	return dynamic_cast<T*>(this->nodeById(strNodeId));
}

template<typename T>
inline T * QUaServer::browsePath(const QStringList & strBrowsePath)
{
	return dynamic_cast<T*>(this->browsePath(strBrowsePath));
}

// -------- OTHER TYPES --------------------------------------------------

template<typename T>
inline T * QUaBaseObject::addChild(const QString &strNodeId/* = ""*/)
{
    return m_qUaServer->createInstance<T>(this, strNodeId);
}

template<typename T>
inline T * QUaBaseDataVariable::addChild(const QString &strNodeId/* = ""*/)
{
    return m_qUaServer->createInstance<T>(this, strNodeId);
}

template<typename T>
inline void QUaBaseVariable::setDataTypeEnum()
{
	// register if not registered
	m_qUaServer->registerEnum<T>();
	this->setDataTypeEnum(QMetaEnum::fromType<T>());
}

template <typename ClassType, typename R, bool IsMutable, typename... Args>
struct QUaMethodTraitsBase
{
    inline static bool getIsMutable()
    {
        return IsMutable;
    }

    inline static const size_t getNumArgs()
    {
        return sizeof...(Args);
    }

    template<typename T>
    inline static QString getTypeName()
    {
        return QString(typeid(T).name());
    }

    inline static QString getRetType()
    {
        return getTypeName<R>();
    }

    inline static QStringList getArgTypes()
    {
        return { getTypeName<Args>()... };
    }

    // https://stackoverflow.com/questions/6627651/enable-if-method-specialization
    template<typename T>
    inline static UA_Argument getTypeUaArgument(QUaServer * uaServer, const int &iArg = 0)
    {
        return getTypeUaArgumentInternal<T>(std::is_enum<T>(), uaServer, iArg);
    }

    template<typename T>
    inline static UA_Argument getTypeUaArgumentInternal(std::false_type, QUaServer * uaServer, const int &iArg = 0)
    {
        Q_UNUSED(uaServer);
        UA_NodeId nodeId = QUaTypesConverter::uaTypeNodeIdFromCpp<T>();
        return getTypeUaArgumentInternal<T>(nodeId, iArg);
    }

    template<typename T>
    inline static UA_Argument getTypeUaArgumentInternal(std::true_type, QUaServer * uaServer, const int &iArg = 0)
    {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        // compose enum name
        QString strEnumName = QString("%1::%2").arg(metaEnum.scope()).arg(metaEnum.enumName());
        // register if not exists
        if (!uaServer->m_hashEnums.contains(strEnumName))
        {
            uaServer->registerEnum(metaEnum);
        }
        Q_ASSERT(uaServer->m_hashEnums.contains(strEnumName));
        // pass in enum nodeid
        UA_NodeId nodeId = uaServer->m_hashEnums.value(strEnumName);
        return getTypeUaArgumentInternal<T>(nodeId, iArg);
    }

    template<typename T>
    inline static UA_Argument getTypeUaArgumentInternal(const UA_NodeId &nodeId, const int &iArg = 0)
    {
        UA_Argument inputArgument;
        UA_Argument_init(&inputArgument);
        // create n-th argument with name "Arg" + number
        inputArgument.description = UA_LOCALIZEDTEXT((char *)"", (char *)"Method Argument");
        inputArgument.name        = QUaTypesConverter::uaStringFromQString(QObject::trUtf8("Arg%1").arg(iArg));
        inputArgument.dataType    = nodeId;
        inputArgument.valueRank   = UA_VALUERANK_SCALAR; // TODO : support arrays UA_VALUERANK_ANY?
        // return
        return inputArgument;
    }

    inline static const bool isRetUaArgumentVoid()
    {
        return std::is_same<R, void>::value;
    }

    inline static UA_Argument getRetUaArgument()
    {
        if (isRetUaArgumentVoid()) return UA_Argument();
        // create output argument
        UA_Argument outputArgument;
        UA_Argument_init(&outputArgument);
        outputArgument.description = UA_LOCALIZEDTEXT((char *)"",
                                                      (char *)"Result Value");
        outputArgument.name        = QUaTypesConverter::uaStringFromQString((char *)"Result");
        outputArgument.dataType    = QUaTypesConverter::uaTypeNodeIdFromCpp<R>();
        outputArgument.valueRank   = UA_VALUERANK_SCALAR;
        return outputArgument;
    }

    inline static QVector<UA_Argument> getArgsUaArguments(QUaServer * uaServer)
    {
        int iArg = 0;
        const size_t nArgs = getNumArgs();
        if (nArgs <= 0) return QVector<UA_Argument>();
        return { getTypeUaArgument<Args>(uaServer, iArg++)... };
    }

    template<typename T>
    inline static T convertArgType(const UA_Variant * input, const int &iArg)
    {
        QVariant varQt = QUaTypesConverter::uaVariantToQVariant(input[iArg]);
        return varQt.value<T>();
    }

    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
    {
        // call method
        // NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
        int iArg = (int)getNumArgs() - 1;
        // call method
        QVariant varResult = methodCallback(convertArgType<Args>(input, iArg--)...);
        // set result
        UA_Variant retVar = QUaTypesConverter::uaVariantFromQVariant(varResult);

        // TODO : cleanup? UA_Variant_deleteMembers(&retVar);

        return retVar;
    }
};
// general case
template<typename T>
struct QOpcUaMethodTraits : QOpcUaMethodTraits<decltype(&T::operator())>
{};
// specialization - const
template <typename ClassType, typename R, typename... Args>
struct QOpcUaMethodTraits< R(ClassType::*)(Args...) const > : QUaMethodTraitsBase<ClassType, R, false, Args...>
{};
// specialization - mutable
template <typename ClassType, typename R, typename... Args>
struct QOpcUaMethodTraits< R(ClassType::*)(Args...) > : QUaMethodTraitsBase<ClassType, R, true, Args...>
{};
// specialization - const | no return value
template <typename ClassType, typename... Args>
struct QOpcUaMethodTraits< void(ClassType::*)(Args...) const > : QUaMethodTraitsBase<ClassType, void, false, Args...>
{
    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
    {
        // call method
        // NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
        int iArg = (int)QOpcUaMethodTraits<M>::getNumArgs() - 1;
        // call method
        methodCallback(QOpcUaMethodTraits<M>::template convertArgType<Args>(input, iArg--)...);
        // set result
        return UA_Variant();
    }
};
// specialization - mutable | no return value
template <typename ClassType, typename... Args>
struct QOpcUaMethodTraits< void(ClassType::*)(Args...) > : QUaMethodTraitsBase<ClassType, void, true, Args...>
{
    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
    {
        // call method
        // NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
        int iArg = QOpcUaMethodTraits<M>::getNumArgs() - 1;
        // call method
        methodCallback(QOpcUaMethodTraits<M>::template convertArgType<Args>(input, iArg--)...);
        // set result
        return UA_Variant();
    }
};
// specialization - function pointer
template <typename R, typename... Args>
struct QOpcUaMethodTraits< R(*)(Args...) > : QUaMethodTraitsBase<void, R, true, Args...>
{};
// specialization - function pointer | no return value
template <typename... Args>
struct QOpcUaMethodTraits< void(*)(Args...) > : QUaMethodTraitsBase<void, void, true, Args...>
{
    template<typename M>
    inline static UA_Variant execCallback(const M &methodCallback, const UA_Variant * input)
    {
        // call method
        // NOTE : arguments inverted when calling "methodCallback"? only x++ and x-- work (i.e. not --x)?
        int iArg = QOpcUaMethodTraits<M>::getNumArgs() - 1;
        // call method
        methodCallback(QOpcUaMethodTraits<M>::template convertArgType<Args>(input, iArg--)...);
        // set result
        return UA_Variant();
    }
};

template<typename M>
inline void QUaBaseObject::addMethod(const QString & strMethodName, const M & methodCallback)
{
    // create input arguments
    UA_Argument * p_inputArguments = nullptr;
    QVector<UA_Argument> listInputArguments;
    if (QOpcUaMethodTraits<M>::getNumArgs() > 0)
    {
        listInputArguments = QOpcUaMethodTraits<M>::getArgsUaArguments(m_qUaServer);
        p_inputArguments = listInputArguments.data();
    }
    // create output arguments
    UA_Argument * p_outputArgument = nullptr;
    UA_Argument outputArgument;
    if (!QOpcUaMethodTraits<M>::isRetUaArgumentVoid())
    {
        outputArgument = QOpcUaMethodTraits<M>::getRetUaArgument();
        p_outputArgument = &outputArgument;
    }
    // add method node
    QByteArray byteMethodName = strMethodName.toUtf8();
    UA_NodeId methNodeId = this->addMethodNodeInternal(
        byteMethodName,
        QOpcUaMethodTraits<M>::getNumArgs(),
        p_inputArguments,
        p_outputArgument
    );
    // store method with node id hash as key
    Q_ASSERT_X(!m_hashMethods.contains(methNodeId), "QUaBaseObject::addMethodInternal", "Method already exists, callback will be overwritten.");
    m_hashMethods[methNodeId] = [methodCallback](const UA_Variant * input, UA_Variant * output) {
        // call method
        if (QOpcUaMethodTraits<M>::isRetUaArgumentVoid())
        {
            QOpcUaMethodTraits<M>::execCallback(methodCallback, input);
        }
        else
        {
            *output = QOpcUaMethodTraits<M>::execCallback(methodCallback, input);
        }
        // return success status
        return UA_STATUSCODE_GOOD;
    };
}

#endif // QUASERVER_H
