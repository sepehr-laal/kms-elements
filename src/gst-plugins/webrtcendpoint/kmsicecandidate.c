/*
 * (C) Copyright 2015 Kurento (http://kurento.org/)
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the GNU Lesser General Public License
 * (LGPL) version 2.1 which accompanies this distribution, and is available at
 * http://www.gnu.org/licenses/lgpl-2.1.html
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "kmsicecandidate.h"
#include <gst/gst.h>

#define GST_CAT_DEFAULT kmsicecandidate
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);
#define GST_DEFAULT_NAME "kmsicecandidate"

#define kms_ice_candidate_parent_class parent_class
G_DEFINE_TYPE (KmsIceCandidate, kms_ice_candidate, G_TYPE_OBJECT);

#define KMS_ICE_CANDIDATE_GET_PRIVATE(obj) (    \
  G_TYPE_INSTANCE_GET_PRIVATE (                 \
    (obj),                                      \
    KMS_TYPE_ICE_CANDIDATE,                     \
    KmsIceCandidatePrivate                      \
  )                                             \
)

#define DEFAULT_CANDIDATE    NULL
#define DEFAULT_SDP_MID    NULL
#define DEFAULT_SDP_M_LINE_INDEX    0

enum
{
  PROP_0,
  PROP_CANDIDATE,
  PROP_SDP_MID,
  PROP_SDP_M_LINE_INDEX,
  N_PROPERTIES
};

/* Based on http://www.w3.org/TR/webrtc/#rtcicecandidate-type */
struct _KmsIceCandidatePrivate
{
  gchar *candidate;
  gchar *sdp_mid;
  guint8 sdp_m_line_index;
};

static void
kms_ice_candidate_set_property (GObject * gobject, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  KmsIceCandidate *self = KMS_ICE_CANDIDATE (gobject);

  switch (property_id) {
    case PROP_CANDIDATE:{
      const gchar *str = g_value_get_string (value);

      g_free (self->priv->candidate);
      self->priv->candidate = g_strdup (str);
      break;
    }
    case PROP_SDP_MID:{
      const gchar *str = g_value_get_string (value);

      g_free (self->priv->sdp_mid);
      self->priv->sdp_mid = g_strdup (str);
      break;
    }
    case PROP_SDP_M_LINE_INDEX:
      self->priv->sdp_m_line_index = g_value_get_uint (value);
      break;
  }
}

static void
kms_bse_rtp_endpoint_get_property (GObject * gobject, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  KmsIceCandidate *self = KMS_ICE_CANDIDATE (gobject);

  switch (property_id) {
    case PROP_CANDIDATE:
      g_value_set_string (value, self->priv->candidate);
      break;
    case PROP_SDP_MID:
      g_value_set_string (value, self->priv->sdp_mid);
      break;
    case PROP_SDP_M_LINE_INDEX:
      g_value_set_uint (value, self->priv->sdp_m_line_index);
      break;
  }
}

static void
kms_ice_candidate_finalize (GObject * gobject)
{
  KmsIceCandidate *self = KMS_ICE_CANDIDATE (gobject);

  GST_DEBUG_OBJECT (self, "finalize");

  g_free (self->priv->candidate);
  g_free (self->priv->sdp_mid);

  G_OBJECT_CLASS (kms_ice_candidate_parent_class)->finalize (gobject);
}

static void
kms_ice_candidate_class_init (KmsIceCandidateClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = kms_ice_candidate_finalize;
  gobject_class->set_property = kms_ice_candidate_set_property;
  gobject_class->get_property = kms_bse_rtp_endpoint_get_property;

  GST_DEBUG_CATEGORY_INIT (GST_CAT_DEFAULT, GST_DEFAULT_NAME, 0,
      GST_DEFAULT_NAME);

  g_object_class_install_property (gobject_class, PROP_CANDIDATE,
      g_param_spec_string ("candidate",
          "ICE candidate with string representation",
          "The candidate-attribute as defined in section 15.1 of ICE (rfc5245).",
          DEFAULT_CANDIDATE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDP_MID,
      g_param_spec_string ("sdp-mid", "ID of the related m-line",
          "If present, this contains the identifier of the 'media stream identification' "
          "as defined in [RFC 3388] for the m-line this candidate is associated with.",
          DEFAULT_SDP_MID, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class, PROP_SDP_M_LINE_INDEX,
      g_param_spec_uint ("sdp-m-line-index", "Index of the related m-line",
          "The index (starting at zero) of the m-line in the SDP this candidate is associated with.",
          0, G_MAXUINT8, DEFAULT_SDP_M_LINE_INDEX,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  g_type_class_add_private (klass, sizeof (KmsIceCandidatePrivate));
}

static void
kms_ice_candidate_init (KmsIceCandidate * self)
{
  self->priv = KMS_ICE_CANDIDATE_GET_PRIVATE (self);

  self->priv->candidate = DEFAULT_CANDIDATE;
  self->priv->sdp_mid = DEFAULT_SDP_MID;
  self->priv->sdp_m_line_index = DEFAULT_SDP_M_LINE_INDEX;
}

KmsIceCandidate *
kms_ice_candidate_new_from_nice (NiceAgent * agent, NiceCandidate * candidate,
    const gchar * sdp_mid, guint8 sdp_m_line_index)
{
  KmsIceCandidate *obj;
  gchar *str, *cand;

  str = nice_agent_generate_local_candidate_sdp (agent, candidate);
  cand = g_strconcat (SDP_CANDIDATE_ATTR, ":",
      (str + SDP_CANDIDATE_ATTR_LEN), NULL);
  g_free (str);

  obj = g_object_new (KMS_TYPE_ICE_CANDIDATE, "candidate", cand,
      "sdp-mid", sdp_mid, "sdp-m-line-index", sdp_m_line_index, NULL);
  g_free (cand);

  return obj;
}

KmsIceCandidate *
kms_ice_candidate_new (const gchar * candidate,
    const gchar * sdp_mid, guint8 sdp_m_line_index)
{
  return g_object_new (KMS_TYPE_ICE_CANDIDATE, "candidate", candidate,
      "sdp-mid", sdp_mid, "sdp-m-line-index", sdp_m_line_index, NULL);
}