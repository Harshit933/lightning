#ifndef LIGHTNING_DAEMON_WATCH_H
#define LIGHTNING_DAEMON_WATCH_H
#include "config.h"
#include "bitcoin/shadouble.h"
#include <ccan/crypto/ripemd160/ripemd160.h>
#include <ccan/htable/htable_type.h>
#include <ccan/list/list.h>
#include <ccan/short_types/short_types.h>
#include <ccan/typesafe_cb/typesafe_cb.h>

struct bitcoin_tx;
struct lightningd_state;

struct txwatch_output {
	struct sha256_double txid;
	unsigned int index;
};

/* Watching an output */
struct txowatch {
	/* Peer who owns us. */
	struct peer *peer;
	
	/* Output to watch. */
	struct txwatch_output out;

	/* A new tx. */
	void (*cb)(struct peer *peer,
		   const struct bitcoin_tx *tx,
		   void *cbdata);

	void *cbdata;
};

const struct txwatch_output *txowatch_keyof(const struct txowatch *w);
size_t txo_hash(const struct txwatch_output *out);
bool txowatch_eq(const struct txowatch *w, const struct txwatch_output *out);

HTABLE_DEFINE_TYPE(struct txowatch, txowatch_keyof, txo_hash, txowatch_eq,
		   txowatch_hash);

struct txwatch {
	struct lightningd_state *dstate;

	/* Peer who owns us. */
	struct peer *peer;
	
	/* Transaction to watch. */
	struct sha256_double txid;
	int depth;

	/* A new depth (-1 if conflicted), blkhash valid if > 0 */
	void (*cb)(struct peer *peer, int depth,
		   const struct sha256_double *blkhash,
		   const struct sha256_double *txid,
		   void *cbdata);
	void *cbdata;
};

const struct sha256_double *txwatch_keyof(const struct txwatch *w);
size_t txid_hash(const struct sha256_double *txid);
bool txwatch_eq(const struct txwatch *w, const struct sha256_double *txid);
HTABLE_DEFINE_TYPE(struct txwatch, txwatch_keyof, txid_hash, txwatch_eq,
		   txwatch_hash);


struct txwatch *watch_txid_(const tal_t *ctx,
			    struct peer *peer,
			    const struct sha256_double *txid,
			    void (*cb)(struct peer *peer, int depth,
				       const struct sha256_double *blkhash,
				       const struct sha256_double *txidx,
				       void *),
			    void *cbdata);

#define watch_txid(ctx, peer, txid, cb, cbdata)				\
	watch_txid_((ctx), (peer), (txid),				\
		    typesafe_cb_preargs(void, void *,			\
					(cb), (cbdata),			\
					struct peer *,			\
					int depth,			\
					const struct sha256_double *,	\
					const struct sha256_double *txidx), \
		    (cbdata))

struct txwatch *watch_tx_(const tal_t *ctx,
			  struct peer *peer,
			  const struct bitcoin_tx *tx,
			  void (*cb)(struct peer *peer, int depth,
				     const struct sha256_double *blkhash,
				     const struct sha256_double *txidx,
				     void *),
			  void *cbdata);

#define watch_tx(ctx, peer, tx, cb, cbdata)				\
	watch_tx_((ctx), (peer), (tx),					\
		  typesafe_cb_preargs(void, void *,			\
				      (cb), (cbdata),			\
				      struct peer *,			\
				      int depth,			\
				      const struct sha256_double *,	\
				      const struct sha256_double *),	\
		  (cbdata))

struct txowatch *watch_txo_(const tal_t *ctx,
			    struct peer *peer,
			    const struct sha256_double *txid,
			    unsigned int output,
			    void (*cb)(struct peer *peer,
				       const struct bitcoin_tx *tx,
				       void *),
			    void *cbdata);

#define watch_txo(ctx, peer, txid, outnum, cb, cbdata)			\
	watch_txo_((ctx), (peer), (txid), (outnum),			\
		  typesafe_cb_preargs(void, void *,			\
				      (cb), (cbdata),			\
				      struct peer *,			\
				      const struct bitcoin_tx *),	\
		  (cbdata))

void peer_watch_setup(struct peer *peer);

/* FIXME: Seg witness removes need for this! */
void normalized_txid(const struct bitcoin_tx *tx, struct sha256_double *txid);

void setup_watch_timer(struct lightningd_state *dstate);
#endif /* LIGHTNING_DAEMON_WATCH_H */
