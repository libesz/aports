/* apk_database.h - Alpine Package Keeper (APK)
 *
 * Copyright (C) 2005-2008 Natanael Copa <n@tanael.org>
 * Copyright (C) 2008 Timo Teräs <timo.teras@iki.fi>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation. See http://www.gnu.org/ for details.
 */

#ifndef APK_PKG_H
#define APK_PKG_H

#include "apk_version.h"
#include "apk_hash.h"
#include "apk_io.h"

struct apk_database;
struct apk_name;

#define APK_SCRIPT_INVALID		-1
#define APK_SCRIPT_PRE_INSTALL		1
#define APK_SCRIPT_POST_INSTALL		2
#define APK_SCRIPT_PRE_DEINSTALL	3
#define APK_SCRIPT_POST_DEINSTALL	4
#define APK_SCRIPT_PRE_UPGRADE		5
#define APK_SCRIPT_POST_UPGRADE		6

#define APK_PKG_NOT_INSTALLED		0
#define APK_PKG_INSTALLED		1

#define APK_SIGN_NONE			0
#define APK_SIGN_VERIFY			1
#define APK_SIGN_VERIFY_IDENTITY	2
#define APK_SIGN_GENERATE_V1		3
#define APK_SIGN_GENERATE		4
#define APK_SIGN_VERIFY_AND_GENERATE	5

struct apk_sign_ctx {
	int action;
	const EVP_MD *md;
	int num_signatures;
	int control_started : 1;
	int data_started : 1;
	int has_data_checksum : 1;
	int control_verified : 1;
	int data_verified : 1;
	char data_checksum[EVP_MAX_MD_SIZE];
	struct apk_checksum identity;
	EVP_MD_CTX mdctx;

	struct {
		apk_blob_t data;
		EVP_PKEY *pkey;
		char *identity;
	} signature;
};

struct apk_script {
	struct hlist_node script_list;
	unsigned int type;
	unsigned int size;
	char script[];
};

#define APK_DEPMASK_REQUIRE	(APK_VERSION_EQUAL|APK_VERSION_LESS|\
				 APK_VERSION_GREATER)
#define APK_DEPMASK_CONFLICT	(0)

struct apk_dependency {
	struct apk_name *name;
	int result_mask;
	char *version;
};
APK_ARRAY(apk_dependency_array, struct apk_dependency);

struct apk_package {
	apk_hash_node hash_node;

	unsigned repos;
	struct apk_name *name;
	char *version;
	char *url, *description, *license;
	struct apk_dependency_array *depends;
	size_t installed_size, size;
	char *filename;
	struct apk_checksum csum;

	/* for installed packages only */
	struct list_head installed_pkgs_list;
	struct hlist_head owned_dirs;
	struct hlist_head scripts;
};
APK_ARRAY(apk_package_array, struct apk_package *);

extern const char *apk_script_types[];

void apk_sign_ctx_init(struct apk_sign_ctx *ctx, int action,
		       struct apk_checksum *identity);
void apk_sign_ctx_free(struct apk_sign_ctx *ctx);
int apk_sign_ctx_process_file(struct apk_sign_ctx *ctx,
			      const struct apk_file_info *fi,
			      struct apk_istream *is);
int apk_sign_ctx_verify_tar(void *ctx, const struct apk_file_info *fi,
			    struct apk_istream *is);
int apk_sign_ctx_mpart_cb(void *ctx, int part, apk_blob_t blob);

int apk_dep_from_blob(struct apk_dependency *dep, struct apk_database *db,
		      apk_blob_t blob);
void apk_dep_from_pkg(struct apk_dependency *dep, struct apk_database *db,
		      struct apk_package *pkg);
int apk_deps_add(struct apk_dependency_array **depends,
		 struct apk_dependency *dep);
void apk_deps_del(struct apk_dependency_array **deps,
		  struct apk_name *name);
void apk_deps_parse(struct apk_database *db,
		    struct apk_dependency_array **depends,
		    apk_blob_t blob);

int apk_deps_write(struct apk_dependency_array *deps, struct apk_ostream *os);
int apk_script_type(const char *name);

struct apk_package *apk_pkg_new(void);
int apk_pkg_read(struct apk_database *db, const char *name,
		 struct apk_sign_ctx *ctx, struct apk_package **pkg);
void apk_pkg_free(struct apk_package *pkg);

int apk_pkg_parse_name(apk_blob_t apkname, apk_blob_t *name, apk_blob_t *version);

int apk_pkg_add_info(struct apk_database *db, struct apk_package *pkg,
		     char field, apk_blob_t value);
int apk_pkg_get_state(struct apk_package *pkg);
void apk_pkg_set_state(struct apk_database *db, struct apk_package *pkg, int state);
int apk_pkg_add_script(struct apk_package *pkg, struct apk_istream *is,
		       unsigned int type, unsigned int size);
int apk_pkg_run_script(struct apk_package *pkg, int root_fd,
		       unsigned int type);

struct apk_package *apk_pkg_parse_index_entry(struct apk_database *db, apk_blob_t entry);
int apk_pkg_write_index_entry(struct apk_package *pkg, struct apk_ostream *os);

int apk_pkg_version_compare(struct apk_package *a, struct apk_package *b);

#endif
