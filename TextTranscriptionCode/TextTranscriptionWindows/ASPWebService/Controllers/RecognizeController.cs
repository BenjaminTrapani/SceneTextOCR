using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.Mvc;
using System.Web.Http;
using TextTranscriptionCLRComp;
using System.Web.Routing;
using System.Collections.Specialized;
using System.Net.Http;
using Newtonsoft.Json.Linq;
using System.Threading.Tasks;
using System.Threading;

namespace ASPWebService.Controllers
{
    public class RecognizeController : ApiController
    {
		static TextTranscriptionService transcriptionService;

		public class RecoRequest
		{
			public String base64image;
		}

		public String Post([FromBody]RecoRequest recoRequest)
		{
			if (transcriptionService == null)
			{
				HttpContextBase httpContext = new HttpContextWrapper(HttpContext.Current);
				var urlResource = UrlHelper.GenerateContentUrl("~/Content/ResNetCRNNNewCNTK32SeqLenOut", httpContext);
				string filePath = System.Web.Hosting.HostingEnvironment.MapPath(urlResource);
				transcriptionService = new TextTranscriptionService(filePath);
			}

			byte[] convertedBytes = System.Convert.FromBase64String(recoRequest.base64image);

			// Work around stack overflow errors
			String result = null;
			var thread = new Thread(() =>
			{
				result = transcriptionService.recognizeImage(convertedBytes);
			}, 4194304);
			thread.Start();
			thread.Join();

			return result;
		}
	}
}