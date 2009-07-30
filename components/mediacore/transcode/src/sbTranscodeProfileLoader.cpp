/* vim: set sw=2 : */
/*
//
// BEGIN SONGBIRD GPL
//
// This file is part of the Songbird web player.
//
// Copyright(c) 2005-2009 POTI, Inc.
// http://songbirdnest.com
//
// This file may be licensed under the terms of of the
// GNU General Public License Version 2 (the "GPL").
//
// Software distributed under the License is distributed
// on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
// express or implied. See the GPL for the specific language
// governing rights and limitations.
//
// You should have received a copy of the GPL along with this
// program. If not, go to http://www.gnu.org/licenses/gpl.html
// or write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
// END SONGBIRD GPL
//
*/

#include "sbTranscodeProfileLoader.h"

#include <nsIDOM3Node.h>
#include <nsIDOMDocument.h>
#include <nsIDOMElement.h>
#include <nsIDOMNode.h>
#include <nsIDOMParser.h>
#include <nsIFile.h>
#include <nsIFileStreams.h>
#include <nsIMutableArray.h>

#include <nsAutoPtr.h>
#include <nsCOMPtr.h>
#include <nsComponentManagerUtils.h>
#include <nsStringGlue.h>
#include <nsThreadUtils.h>

#include <sbVariantUtils.h>

#include "sbTranscodeProfile.h"
#include "sbTranscodeProfileProperty.h"

NS_IMPL_THREADSAFE_ISUPPORTS2(sbTranscodeProfileLoader,
                              sbITranscodeProfileLoader,
                              nsIRunnable);

sbTranscodeProfileLoader::sbTranscodeProfileLoader()
{
}

sbTranscodeProfileLoader::~sbTranscodeProfileLoader()
{
}

/* sbITranscodeProfile loadProfile (in nsIFile aFile); */
NS_IMETHODIMP
sbTranscodeProfileLoader::LoadProfile(nsIFile *aFile, sbITranscodeProfile **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  nsresult rv;

  mFile = aFile;
  if (NS_IsMainThread()) {
    // this is on the main thread, call directly
    rv = LoadProfileInternal();
    NS_ENSURE_SUCCESS(rv, rv);
    mProfile.forget(_retval);
  } else {
    nsCOMPtr<nsIRunnable> runnable =
      do_QueryInterface(NS_ISUPPORTS_CAST(nsIRunnable*, this), &rv);
    NS_ASSERTION(runnable, "sbTranscodeProfileLoader should implement nsIRunnable");
    rv = NS_DispatchToMainThread(runnable, NS_DISPATCH_SYNC);
    NS_ENSURE_SUCCESS(rv, rv);

    mProfile.forget(_retval);
    // check the return value from LoadProfileInternal
    NS_ENSURE_SUCCESS(mResult, mResult);
  }
  mFile = nsnull;
  return NS_OK;
}

/** nsIRunnable */
NS_IMETHODIMP
sbTranscodeProfileLoader::Run()
{
  mResult = LoadProfileInternal();
  return NS_OK;
}

nsresult
sbTranscodeProfileLoader::LoadProfileInternal()
{
  const PRInt32 IOFLAGS_DEFAULT = -1;
  const PRInt32 PERMISSIONS_DEFAULT = -1;
  const PRInt32 FLAGS_DEFAULT = 0;

  nsresult rv;

  NS_ENSURE_TRUE(NS_IsMainThread(), NS_ERROR_UNEXPECTED);
  NS_ENSURE_ARG_POINTER(mFile);

  nsCOMPtr<nsIDOMParser> domParser =
    do_CreateInstance("@mozilla.org/xmlextras/domparser;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIFileInputStream> fileStream =
    do_CreateInstance("@mozilla.org/network/file-input-stream;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = fileStream->Init(mFile,
                        IOFLAGS_DEFAULT,
                        PERMISSIONS_DEFAULT,
                        FLAGS_DEFAULT);

  PRUint32 fileSize;
  rv = fileStream->Available(&fileSize);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMDocument> doc;
  rv = domParser->ParseFromStream(fileStream,
                                  nsnull,
                                  fileSize,
                                  "text/xml",
                                  getter_AddRefs(doc));
  NS_ENSURE_SUCCESS(rv, rv);

  rv = fileStream->Close();
  NS_ENSURE_SUCCESS(rv, rv);

  mProfile = do_CreateInstance(SONGBIRD_TRANSCODEPROFILE_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMElement> element;
  rv = doc->GetDocumentElement(getter_AddRefs(element));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMNode> childNode;
  rv = element->GetFirstChild(getter_AddRefs(childNode));
  NS_ENSURE_SUCCESS(rv, rv);

  while (childNode) {
    nsCOMPtr<nsIDOMElement> childElement = do_QueryInterface(childNode, &rv);

    if (NS_SUCCEEDED(rv)) {
      nsString localName;
      rv = childElement->GetLocalName(localName);
      NS_ENSURE_SUCCESS(rv, rv);

      if (localName.EqualsLiteral("type")) {
        PRUint32 type;
        rv = GetType(childNode, &type);
        NS_ENSURE_SUCCESS(rv, rv);
        rv = mProfile->SetType(type);
        NS_ENSURE_SUCCESS(rv, rv);
      } else if (localName.EqualsLiteral("description")) {
        nsCOMPtr<nsIDOM3Node> dom3Node = do_QueryInterface(childNode);
        if (dom3Node) {
          nsString textContent;
          rv = dom3Node->GetTextContent(textContent);
          NS_ENSURE_SUCCESS(rv, rv);
          rv = mProfile->SetDescription(textContent);
          NS_ENSURE_SUCCESS(rv, rv);
        }
      } else if (localName.EqualsLiteral("id")) {
        nsCOMPtr<nsIDOM3Node> dom3Node = do_QueryInterface(childNode);
        if (dom3Node) {
          nsString textContent;
          rv = dom3Node->GetTextContent(textContent);
          NS_ENSURE_SUCCESS(rv, rv);
          rv = mProfile->SetId(textContent);
          NS_ENSURE_SUCCESS(rv, rv);
        }
      } else if (localName.EqualsLiteral("container")) {
        rv = ProcessContainer(mProfile, CONTAINER_GENERIC, childElement);
        NS_ENSURE_SUCCESS(rv, rv);
      } else if (localName.EqualsLiteral("audio")) {
        rv = ProcessContainer(mProfile, CONTAINER_AUDIO, childElement);
        NS_ENSURE_SUCCESS(rv, rv);
      } else if (localName.EqualsLiteral("video")) {
        rv = ProcessContainer(mProfile, CONTAINER_VIDEO, childElement);
        NS_ENSURE_SUCCESS(rv, rv);
      }
    }

    rv = childNode->GetNextSibling(getter_AddRefs(childNode));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return NS_OK;
}

nsresult
sbTranscodeProfileLoader::GetType(nsIDOMNode* aTypeNode, PRUint32* _retval)
{
  NS_ENSURE_ARG_POINTER(aTypeNode);
  NS_ENSURE_ARG_POINTER(_retval);

  nsresult rv;

  nsCOMPtr<nsIDOM3Node> dom3node = do_QueryInterface(aTypeNode, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsString type;
  rv = dom3node->GetTextContent(type);
  NS_ENSURE_SUCCESS(rv, rv);

  if (type.EqualsLiteral("audio")) {
    *_retval = sbITranscodeProfile::TRANSCODE_TYPE_AUDIO;
  } else if (type.EqualsLiteral("video")) {
    *_retval = sbITranscodeProfile::TRANSCODE_TYPE_VIDEO;
  } else if (type.EqualsLiteral("audio+video")) {
    *_retval = sbITranscodeProfile::TRANSCODE_TYPE_AUDIO_VIDEO;
  } else {
    return NS_ERROR_INVALID_ARG;
  }
  return NS_OK;
}

nsresult
sbTranscodeProfileLoader::ProcessProperty(nsIDOMElement* aPropertyElement,
                                          sbITranscodeProfileProperty** _retval)
{
  NS_ENSURE_ARG_POINTER(aPropertyElement);
  NS_ENSURE_ARG_POINTER(_retval);

  nsresult rv;

  nsRefPtr<sbTranscodeProfileProperty> property =
    new sbTranscodeProfileProperty();
  NS_ENSURE_TRUE(property, NS_ERROR_OUT_OF_MEMORY);

  nsString attributeValue;
  rv = aPropertyElement->GetAttribute(NS_LITERAL_STRING("name"),
                                      attributeValue);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = property->SetPropertyName(attributeValue);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = aPropertyElement->GetAttribute(NS_LITERAL_STRING("type"),
                                      attributeValue);
  NS_ENSURE_SUCCESS(rv, rv);

  if (attributeValue.EqualsLiteral("int")) {
    PRInt32 value;

    rv = aPropertyElement->GetAttribute(NS_LITERAL_STRING("min"),
                                        attributeValue);
    NS_ENSURE_SUCCESS(rv, rv);

    value = attributeValue.ToInteger(&rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = property->SetValueMin(sbNewVariant(value));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = aPropertyElement->GetAttribute(NS_LITERAL_STRING("max"),
                                        attributeValue);
    NS_ENSURE_SUCCESS(rv, rv);

    value = attributeValue.ToInteger(&rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = property->SetValueMax(sbNewVariant(value));
    NS_ENSURE_SUCCESS(rv, rv);

    rv = aPropertyElement->GetAttribute(NS_LITERAL_STRING("default"),
                                        attributeValue);
    NS_ENSURE_SUCCESS(rv, rv);

    value = attributeValue.ToInteger(&rv);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = property->SetValue(sbNewVariant(value));
    NS_ENSURE_SUCCESS(rv, rv);
  } else {
    return NS_ERROR_NOT_IMPLEMENTED;
  }

  rv = CallQueryInterface(property.get(), _retval);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

nsresult
sbTranscodeProfileLoader::ProcessContainer(sbITranscodeProfile* aProfile,
                                           ContainerType_t aContainerType,
                                           nsIDOMElement* aContainer)
{
  NS_ENSURE_ARG_POINTER(aProfile);
  NS_ENSURE_ARG_POINTER(aContainer);

  nsresult rv;

  nsCOMPtr<nsIMutableArray> properties =
    do_CreateInstance("@songbirdnest.com/moz/xpcom/threadsafe-array;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMNode> childNode;
  rv = aContainer->GetFirstChild(getter_AddRefs(childNode));
  NS_ENSURE_SUCCESS(rv, rv);

  while (childNode) {
    nsCOMPtr<nsIDOMElement> childElement = do_QueryInterface(childNode);
    if (childElement) {
      nsString localName;
      rv = childElement->GetLocalName(localName);
      NS_ENSURE_SUCCESS(rv, rv);

      if (localName.EqualsLiteral("type")) {
        nsCOMPtr<nsIDOM3Node> dom3Node = do_QueryInterface(childNode, &rv);
        NS_ENSURE_SUCCESS(rv, rv);

        nsString textContent;
        rv = dom3Node->GetTextContent(textContent);
        NS_ENSURE_SUCCESS(rv, rv);

        switch (aContainerType) {
          case CONTAINER_GENERIC:
            rv = aProfile->SetContainerFormat(textContent);
            break;
          case CONTAINER_AUDIO:
            rv = aProfile->SetAudioCodec(textContent);
            break;
          case CONTAINER_VIDEO:
            rv = aProfile->SetVideoCodec(textContent);
            break;
          default:
            rv = NS_ERROR_UNEXPECTED;
        }
        NS_ENSURE_SUCCESS(rv, rv);
      } else if (localName.EqualsLiteral("property")) {
        nsCOMPtr<sbITranscodeProfileProperty> property;
        rv = ProcessProperty(childElement, getter_AddRefs(property));
        NS_ENSURE_SUCCESS(rv, rv);

        rv = properties->AppendElement(property, PR_FALSE);
        NS_ENSURE_SUCCESS(rv, rv);
      }
    }
    rv = childNode->GetNextSibling(getter_AddRefs(childNode));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  switch (aContainerType) {
    case CONTAINER_GENERIC:
      rv = aProfile->SetContainerProperties(properties);
      break;
    case CONTAINER_AUDIO:
      rv = aProfile->SetAudioProperties(properties);
      break;
    case CONTAINER_VIDEO:
      rv = aProfile->SetVideoProperties(properties);
      break;
    default:
      rv = NS_ERROR_UNEXPECTED;
  }
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}
