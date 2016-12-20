// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.
#pragma once

#include <vespa/searchcore/config/config-proton.h>
#include <vespa/searchcore/proton/documentmetastore/i_document_meta_store.h>
#include <vespa/searchcore/proton/matching/matching_stats.h>
#include <vespa/searchcore/proton/reprocessing/i_reprocessing_task.h>
#include <vespa/searchcorespi/flush/iflushtarget.h>
#include <vespa/searchlib/common/serialnum.h>
#include <vespa/searchlib/util/searchable_stats.h>


namespace search {
    namespace index {
        class Schema;
    }
}
namespace document { class DocumentId; }

namespace searchcorespi {
    class IIndexManagerFactory;
    class IIndexManager;
}
namespace proton {
    namespace matching { class SessionManager; }

class FeedHandler;
class DocumentDBConfig;
class FileConfigManager;
class IReplayConfig;
class IIndexWriter;
class IDcoumentRetriever;
class DocumentSubDbInitializer;
class DocumentSubDbInitializerResult;
class ReconfigParams;
class IFeedView;
class ISearchHandler;
class ISummaryManager;
class ISummaryAdapter;
class IDocumentMetaStoreContext;
class IDocumentRetriever;
class IAttributeManager;

/**
 * Interface for a document sub database that handles a subset of the documents that belong to a
 * DocumentDB.
 *
 * Documents can be inserted/updated/removed to a sub database via a feed view,
 * searched via a search view and retrieved via a document retriever.
 * A sub database is separate and independent from other sub databases.
 */
class IDocumentSubDB
{
public:
    class IOwner
    {
    public:
        virtual ~IOwner() {}
        virtual void syncFeedView() = 0;
        virtual std::shared_ptr<searchcorespi::IIndexManagerFactory>
        getIndexManagerFactory(const vespalib::stringref &name) const = 0;
        virtual vespalib::string getName() const = 0;
        virtual uint32_t getDistributionKey() const = 0;
    };

    using UP = std::unique_ptr<IDocumentSubDB>;
    using SerialNum = search::SerialNum;
    using Schema = search::index::Schema;
    using SchemaSP = std::shared_ptr<Schema>;
    using ProtonConfig = vespa::config::search::core::ProtonConfig;
    using IFlushTarget = searchcorespi::IFlushTarget;
public:
    IDocumentSubDB() { }
    virtual ~IDocumentSubDB() { }
    virtual uint32_t getSubDbId() const = 0;
    virtual vespalib::string getName() const = 0;

    virtual std::unique_ptr<DocumentSubDbInitializer>
    createInitializer(const DocumentDBConfig &configSnapshot, SerialNum configSerialNum, const SchemaSP &unionSchema,
                      const ProtonConfig::Summary &protonSummaryCfg, const ProtonConfig::Index &indexCfg) const = 0;

    // Called by master thread
    virtual void setup(const DocumentSubDbInitializerResult &initResult) = 0;
    virtual void initViews(const DocumentDBConfig &configSnapshot, const std::shared_ptr<matching::SessionManager> &sessionManager) = 0;

    virtual IReprocessingTask::List
    applyConfig(const DocumentDBConfig &newConfigSnapshot, const DocumentDBConfig &oldConfigSnapshot,
                SerialNum serialNum, const ReconfigParams & params) = 0;

    virtual std::shared_ptr<ISearchHandler> getSearchView() const = 0;
    virtual std::shared_ptr<IFeedView> getFeedView() const = 0;
    virtual void clearViews() = 0;
    virtual const std::shared_ptr<ISummaryManager> &getSummaryManager() const = 0;
    virtual std::shared_ptr<IAttributeManager> getAttributeManager() const = 0;
    virtual const std::shared_ptr<searchcorespi::IIndexManager> &getIndexManager() const = 0;
    virtual const std::shared_ptr<ISummaryAdapter> &getSummaryAdapter() const = 0;
    virtual const std::shared_ptr<IIndexWriter> &getIndexWriter() const = 0;
    virtual IDocumentMetaStoreContext &getDocumentMetaStoreContext() = 0;
    virtual IFlushTarget::List getFlushTargets() = 0;
    virtual size_t getNumDocs() const = 0;
    virtual size_t getNumActiveDocs() const = 0;
    /**
     * Needed by FeedRouter::handleRemove().
     * TODO: remove together with FeedEngine.
     **/
    virtual bool hasDocument(const document::DocumentId &id) = 0;
    virtual void onReplayDone() = 0;
    virtual void onReprocessDone(SerialNum serialNum) = 0;

    /*
     * Get oldest flushed serial for components.
     */
    virtual SerialNum getOldestFlushedSerial() = 0;

    /*
     * Get newest flushed serial.  Used to validate that we've not lost
     * last part of transaction log.
     */
    virtual SerialNum getNewestFlushedSerial()  = 0;
    virtual void wipeHistory(SerialNum wipeSerial, const Schema &newHistorySchema, const Schema &wipeSchema) = 0;
    virtual void setIndexSchema(const SchemaSP &schema, const SchemaSP &fusionSchema) = 0;
    virtual search::SearchableStats getSearchableStats() const = 0;
    virtual std::unique_ptr<IDocumentRetriever> getDocumentRetriever() = 0;

    virtual matching::MatchingStats getMatcherStats(const vespalib::string &rankProfile) const = 0;
    virtual void close() = 0;
};

} // namespace proton